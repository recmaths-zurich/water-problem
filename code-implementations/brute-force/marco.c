/*
  We use recursion to only check all cases where n+1 cells are filled.
  */

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

static inline void unfillCell(uint64_t *state, int idx) {
    *state &= ~(1ULL << idx);
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

int recurse(uint64_t *state, int i, int depth) {
    int max_length = compute_length(*state);
    
    if (depth > 0) {
        for (int j = i + 1; j < g_n * g_n; j++) {
            fillCell(state, j);
            int length = recurse(state, j, depth - 1);
            if (length > max_length) {
                max_length = length;
            }
            unfillCell(state, j);
        }
    }
    
    if (depth == g_n + 1) {
        printf("i = %d\n", i);
    }

    return max_length;
}

int main(void) {
    printf("Enter grid size (1 to 8): ");
    if (scanf("%d", &g_n) != 1 || g_n < 1 || g_n > 8) {
        printf("Invalid input. Please run again with n between 1 and 8.\n");
        return 1;
    }

    buildNeighborMasks();

    uint64_t initialState = 0ULL;
    int maxLength = recurse(&initialState, -1, g_n + 2);

    printf("Max length = %d\n", maxLength);
    // printf("Best state = %" PRIu64 "\n", bestState);
    // printGrid(bestState);

    return 0;
}
