#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <time.h>      // for time(), difftime()

static int g_n = 0;             
static uint64_t neighborMask[64];

static inline int popcount64(uint64_t x) {
    return __builtin_popcountll(x); // or implement your own
}

static inline int cellIndex(int x, int y) {
    return y * g_n + x;
}

static inline int isFilled(uint64_t state, int idx) {
    return (state >> idx) & 1ULL;
}

static inline void fillCell(uint64_t *state, int idx) {
    *state |= (1ULL << idx);
}

// Build neighbor masks for each cell
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

// ------------------------------
// 1) Rotate 90° (clockwise)
//    (x,y) -> (newX,newY) = (n-1 - y, x)
// ------------------------------
uint64_t rotate90(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = g_n - 1 - y;
                int newY = x;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 2) Rotate 180°
//    (x,y) -> (n-1 - x, n-1 - y)
// ------------------------------
uint64_t rotate180(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = g_n - 1 - x;
                int newY = g_n - 1 - y;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 3) Rotate 270° (clockwise)
//    (x,y) -> (newX,newY) = (y, n-1 - x)
// ------------------------------
uint64_t rotate270(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = y;
                int newY = g_n - 1 - x;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 4) Reflect horizontally (flip top <-> bottom)
//    (x,y) -> (x, n-1 - y)
// ------------------------------
uint64_t reflectHorizontal(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = x;
                int newY = g_n - 1 - y;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 5) Reflect vertically (flip left <-> right)
//    (x,y) -> (n-1 - x, y)
// ------------------------------
uint64_t reflectVertical(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = g_n - 1 - x;
                int newY = y;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 6) Reflect along main diagonal (top-left to bottom-right)
//    (x,y) -> (y,x)
// ------------------------------
uint64_t reflectMainDiag(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                // swap x,y
                int newX = y;
                int newY = x;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// 7) Reflect along anti-diagonal (top-right to bottom-left)
//    (x,y) -> (n-1 - y, n-1 - x)
// ------------------------------
uint64_t reflectAntiDiag(uint64_t state) {
    uint64_t out = 0ULL;
    for (int y = 0; y < g_n; y++) {
        for (int x = 0; x < g_n; x++) {
            int oldPos = cellIndex(x, y);
            if (isFilled(state, oldPos)) {
                int newX = g_n - 1 - y;
                int newY = g_n - 1 - x;
                int newPos = cellIndex(newX, newY);
                fillCell(&out, newPos);
            }
        }
    }
    return out;
}

// ------------------------------
// getCanonicalRep
//
// 1) Generate all 8 transformations of 'state'.
// 2) Take the minimum numerical value among them.
// 3) Return that as the canonical representative.
// ------------------------------
uint64_t getCanonicalRep(uint64_t state) {
    // Compute all 8 transformations
    uint64_t t[8];
    t[0] = state;                      // identity
    t[1] = rotate90(state);
    t[2] = rotate180(state);
    t[3] = rotate270(state);
    t[4] = reflectHorizontal(state);
    t[5] = reflectVertical(state);
    t[6] = reflectMainDiag(state);
    t[7] = reflectAntiDiag(state);

    // Pick the minimum
    uint64_t best = t[0];
    for (int i = 1; i < 8; i++) {
        if (t[i] < best) {
            best = t[i];
        }
    }
    return best;
}

// One iteration step; returns 1 if any cell changed
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

// Compute steps until the grid stabilizes
static int compute_length(uint64_t initialState) {
    uint64_t state = initialState;
    int steps = 1;
    while (iteration_step(&state)) {
        steps++;
    }
    return steps;
}

static void printGrid(uint64_t state) {
    // Top boundary
    printf("+");
    for (int i = 0; i < g_n * 2 + 1; i++) {
        printf("-");
    }
    printf("+\n");

    // Rows
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

    // Bottom boundary
    printf("+");
    for (int i = 0; i < g_n * 2 + 1; i++) {
        printf("-");
    }
    printf("+\n");
}

int main(void) {
    printf("Enter grid size (1 to 8): ");
    if (scanf("%d", &g_n) != 1 || g_n < 1 || g_n > 8) {
        printf("Invalid input. Please run again with n between 1 and 8.\n");
        return 1;
    }

    buildNeighborMasks();

    uint64_t totalStates = (1ULL << (g_n*g_n));
    int maxLength = 0;
    uint64_t bestState = 0ULL;

    // Progress settings
    const uint64_t progressInterval = 10000000ULL; // print progress every 10 million states

    // Timing
    time_t startTime = time(NULL);

    for (uint64_t state = 0ULL; state < totalStates; state++) {
        // Show progress occasionally
        if ((state % progressInterval) == 0ULL && state > 0) {
            time_t now = time(NULL);
            double elapsedSecs = difftime(now, startTime);

            // States per second, skipping if elapsedSecs is zero or extremely small
            if (elapsedSecs > 0.0) {
                double sps        = (double)state / elapsedSecs; // states per second
                double statesLeft = (double)(totalStates - state);
                double secsLeft   = statesLeft / sps;
                double minsLeft   = secsLeft / 60.0;
                double percent    = 100.0 * (double)state / (double)totalStates;

                printf("Progress: %llu / %llu (%.2f%%), approx %.1f minutes left\n",
                       (unsigned long long)state,
                       (unsigned long long)totalStates,
                       percent, minsLeft);  
            } else {
                // If elapsedSecs is still 0, we can't compute time left safely
                printf("Progress: %llu / %llu\n",
                       (unsigned long long)state,
                       (unsigned long long)totalStates);
            }
        }

        uint64_t canonical = getCanonicalRep(state);
        if (state != canonical) {
            // we can skip all noncanonical states
            // since they will be covered by symmetry
            continue;
        }

        int length = compute_length(state);
        if (length > maxLength) {
            maxLength = length;
            bestState = state;
        }
    }

    // Final report
    printf("Max length = %d\n", maxLength);
    printf("Best state = %" PRIu64 "\n", bestState);
    printGrid(bestState);

    return 0;
}
