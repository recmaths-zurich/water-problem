#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>  // For C11 atomics

#ifdef _WIN32
#include <windows.h>  // for Sleep()
#else
#include <unistd.h>   // for sleep() on Linux/macOS
#endif

// ---------------------------------------------------------------------
// Global variables
// ---------------------------------------------------------------------
static int g_n = 0;               // Board size, read from user
static uint64_t neighborMask[64]; // Precomputed neighbor masks
static _Atomic uint64_t processed_count = 0; // How many states have been processed

// ---------------------------------------------------------------------
// Popcount (bit count). If your compiler doesn't have __builtin_popcountll,
// you can implement a custom bit-count.
// ---------------------------------------------------------------------
static inline int popcount64(uint64_t x) {
    return __builtin_popcountll(x);
}

// ---------------------------------------------------------------------
// Indexing, fill-check, fill-set
// ---------------------------------------------------------------------
static inline int cellIndex(int x, int y) {
    return y * g_n + x;
}

static inline int isFilled(uint64_t state, int idx) {
    return (state >> idx) & 1ULL;
}

static inline void fillCell(uint64_t *state, int idx) {
    *state |= (1ULL << idx);
}

// ---------------------------------------------------------------------
// Build neighbor masks for each cell
// ---------------------------------------------------------------------
static void buildNeighborMasks(void) {
    memset(neighborMask, 0, sizeof(neighborMask));
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int c = cellIndex(x, y);
            // Up
            if (y > 0)
                neighborMask[c] |= (1ULL << cellIndex(x, y - 1));
            // Down
            if (y < g_n - 1)
                neighborMask[c] |= (1ULL << cellIndex(x, y + 1));
            // Left
            if (x > 0)
                neighborMask[c] |= (1ULL << cellIndex(x - 1, y));
            // Right
            if (x < g_n - 1)
                neighborMask[c] |= (1ULL << cellIndex(x + 1, y));
        }
    }
}

// ---------------------------------------------------------------------
// One iteration step. Returns 1 if any cell changed, else 0.
// ---------------------------------------------------------------------
static int iteration_step(uint64_t *state) {
    uint64_t old_state = *state;
    int changed = 0;
    for (int c = 0; c < g_n*g_n; c++) {
        if (!isFilled(old_state, c)) {
            int count_neighbors = popcount64(old_state & neighborMask[c]);
            if (count_neighbors >= 2) {
                fillCell(state, c);
                changed = 1;
            }
        }
    }
    return changed;
}

// ---------------------------------------------------------------------
// Compute the water-fill length for a given initial state
// ---------------------------------------------------------------------
static int compute_length(uint64_t initialState) {
    uint64_t state = initialState;
    int steps = 1;
    while (iteration_step(&state)) {
        steps++;
    }
    return steps;
}

// ---------------------------------------------------------------------
// Printing an n×n state (optional)
// ---------------------------------------------------------------------
static void printGrid(uint64_t state) {
    printf("+");
    for (int i = 0; i < g_n*2 + 1; i++)
        printf("-");
    printf("+\n");

    for (int y = 0; y < g_n; y++) {
        printf("|");
        for (int x = 0; x < g_n; x++) {
            int c = cellIndex(x, y);
            if (isFilled(state, c)) {
                printf(" W");
            } else {
                printf(" .");
            }
        }
        printf(" |\n");
    }

    printf("+");
    for (int i = 0; i < g_n*2 + 1; i++)
        printf("-");
    printf("+\n");
}

// ---------------------------------------------------------------------
// ThreadTask struct: each worker thread gets a range [start,end)
// ---------------------------------------------------------------------
typedef struct {
    uint64_t startState;     // inclusive
    uint64_t endState;       // exclusive
    int localMaxLength;
    uint64_t localBestState;
} ThreadTask;

// ---------------------------------------------------------------------
// Worker thread function
// ---------------------------------------------------------------------
void* workerThreadFunc(void* arg) {
    ThreadTask* task = (ThreadTask*)arg;

    int localMax = 0;
    uint64_t localBest = 0ULL;

    for (uint64_t s = task->startState; s < task->endState; s++) {
        int length = compute_length(s);
        if (length > localMax) {
            localMax  = length;
            localBest = s;
        }
        // Increment the global processed_count by 1
        atomic_fetch_add(&processed_count, 1ULL);
    }
    task->localMaxLength = localMax;
    task->localBestState = localBest;
    return NULL;
}

// ---------------------------------------------------------------------
// Progress thread function
// Waits and periodically prints how many states have been processed.
// ---------------------------------------------------------------------
typedef struct {
    uint64_t totalStates;
    int doneFlag; // We'll set this once all workers are joined
} ProgressTask;

void* progressThreadFunc(void* arg) {
    ProgressTask* pt = (ProgressTask*)arg;
    uint64_t total = pt->totalStates;

    while (1) {
        // If the doneFlag is set or we've reached totalStates, break
        uint64_t doneSoFar = atomic_load(&processed_count);
        if (pt->doneFlag || doneSoFar >= total) {
            break;
        }

        double percent = 100.0 * (double)doneSoFar / (double)total;
        printf("Progress: %llu / %llu (%.2f%%)\n",
               (unsigned long long)doneSoFar,
               (unsigned long long)total,
               percent);

        // Sleep a bit so we don't spam output
#ifdef _WIN32
        // On Windows you might use Sleep(2000) from <windows.h> in milliseconds
        Sleep(2000);
#else
        // POSIX sleep in seconds
        sleep(2);
#endif
    }
    return NULL;
}

// ---------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------
int main(void) {
    printf("Enter grid size (1 to 8): ");
    if (scanf("%d", &g_n) != 1 || g_n < 1 || g_n > 8) {
        printf("Invalid input.\n");
        return 1;
    }

    // Choose how many threads to launch
    int threadCount = 4; // or read from user

    // Build neighbor masks
    buildNeighborMasks();

    // Number of total states
    uint64_t totalStates = (1ULL << (g_n * g_n));

    // Create the progress thread
    ProgressTask ptask;
    ptask.totalStates = totalStates;
    ptask.doneFlag    = 0;
    pthread_t progressThread;
    pthread_create(&progressThread, NULL, progressThreadFunc, &ptask);

    // Create worker threads
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t)*threadCount);
    ThreadTask* tasks  = (ThreadTask*)malloc(sizeof(ThreadTask)*threadCount);

    // Divide the state space among threads
    uint64_t chunkSize = totalStates / threadCount;
    uint64_t remainder = totalStates % threadCount;

    uint64_t start = 0ULL;
    for (int i = 0; i < threadCount; i++) {
        uint64_t end = start + chunkSize;
        if (remainder > 0) {
            end++;
            remainder--;
        }
        tasks[i].startState = start;
        tasks[i].endState   = end;
        tasks[i].localMaxLength = 0;
        tasks[i].localBestState = 0ULL;
        pthread_create(&threads[i], NULL, workerThreadFunc, &tasks[i]);
        start = end;
    }

    // Wait for all workers to finish
    int globalMaxLength = 0;
    uint64_t globalBestState = 0ULL;

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
        if (tasks[i].localMaxLength > globalMaxLength) {
            globalMaxLength = tasks[i].localMaxLength;
            globalBestState = tasks[i].localBestState;
        }
    }

    // Tell the progress thread we're done
    ptask.doneFlag = 1;

    // Wait for progress thread to exit
    pthread_join(progressThread, NULL);

    // Print results
    printf("Max length = %d\n", globalMaxLength);
    printf("Best state = %" PRIu64 "\n", globalBestState);
    printGrid(globalBestState);

    free(threads);
    free(tasks);

    return 0;
}
