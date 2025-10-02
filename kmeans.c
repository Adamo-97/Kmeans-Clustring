#include <stdio.h>                      // For FILE, fopen, fprintf, printf
#include <stdlib.h>                     // For malloc, realloc, free, exit, srand, rand
#include <time.h>                       // For time() used to seed RNG
#include <string.h>                     // For memset used in zeroing buffers

// ---------------------------
// Data model and global state
// ---------------------------

struct DataPoint {                      // Struct representing one 2D data point
    float x;                            // X coordinate
    float y;                            // Y coordinate
};                                      // End struct DataPoint

static struct DataPoint *dataPoints = NULL; // Dynamic array holding all points (global)
static int numDataPoints = 0;               // Number of points currently stored (global)

// ---------------------------
// File I/O: read and cleanup
// ---------------------------

void readData(char *filename) {         // Function to read data; parameter kept to match your signature
    FILE *dataFile = fopen(filename, "r"); // Open the required input file for reading
    if (!dataFile) {                    // If opening failed
        printf("Failed to open %s", filename); // Report failure
        exit(0);                        // Exit (mirrors your original behavior)
    }
    float x, y;                         // Temporaries to parse each line's x and y
    while (fscanf(dataFile, "%f %f", &x, &y) == 2) { // Keep reading pairs of floats until EOF or parse error
        struct DataPoint *resized =     // Attempt to grow the array by one element
            realloc(dataPoints, (numDataPoints + 1) * sizeof(struct DataPoint));
        if (!resized) {                 // If memory allocation failed
            perror("realloc");          // Print OS error
            break;                      // Stop reading more data
        }
        dataPoints = resized;           // Commit the resized pointer
        dataPoints[numDataPoints].x = x; // Store parsed x in the new slot
        dataPoints[numDataPoints].y = y; // Store parsed y in the new slot
        numDataPoints++;                // Increase the count of stored points
    }
    fclose(dataFile);                   // Close the input file
    printf("Total data points read: %d\n", numDataPoints); // Report how many points were loaded
}

void freeData(void) {                   // Function to free global point storage
    free(dataPoints);                   // Free the dynamic array (safe even if NULL)
    dataPoints = NULL;                  // Reset pointer to NULL to avoid dangling
    numDataPoints = 0;                  // Reset the count to zero
}

// ---------------------------------------
// Internal helpers used by K-means logic
// ---------------------------------------

// Scratch buffers for recompute_centroids; allocated once and reused
static float *sumX_scratch = NULL;      // Per-cluster accumulator for sum of X
static float *sumY_scratch = NULL;      // Per-cluster accumulator for sum of Y
static int   *count_scratch = NULL;     // Per-cluster count of assigned points
static int    scratchK = 0;             // Current capacity (number of clusters) of scratch buffers

static void ensure_scratch(int clusterCount) { // Ensure scratch buffers are allocated to size >= clusterCount
    if (clusterCount != scratchK) {     // If size mismatch with current buffers
        free(sumX_scratch);             // Free old sumX buffer (if any)
        free(sumY_scratch);             // Free old sumY buffer (if any)
        free(count_scratch);            // Free old count buffer (if any)
        sumX_scratch = (float *)malloc((size_t)clusterCount * sizeof *sumX_scratch); // Allocate new sumX
        sumY_scratch = (float *)malloc((size_t)clusterCount * sizeof *sumY_scratch); // Allocate new sumY
        count_scratch = (int   *)malloc((size_t)clusterCount * sizeof *count_scratch); // Allocate new counts
        if (!sumX_scratch || !sumY_scratch || !count_scratch) { // Check allocations
            perror("malloc");           // Report allocation error
            exit(1);                    // Abort program (hard failure)
        }
        scratchK = clusterCount;        // Record new buffer size
    }
}

static void free_scratch(void) {        // Free the scratch buffers (valgrind-clean)
    free(sumX_scratch);                 // Release sumX buffer
    free(sumY_scratch);                 // Release sumY buffer
    free(count_scratch);                // Release counts buffer
    sumX_scratch = NULL;                // Null out pointer
    sumY_scratch = NULL;                // Null out pointer
    count_scratch = NULL;               // Null out pointer
    scratchK = 0;                       // Reset capacity
}

static void pick_k_random_distinct_indices(int totalPoints, int clusterCount, int *outIndices) {
    int *perm = (int *)malloc((size_t)totalPoints * sizeof *perm); // Allocate permutation 0..totalPoints-1
    if (!perm) {                          // Check allocation
        perror("malloc");                 // Report error
        exit(1);                          // Abort
    }
    for (int i = 0; i < totalPoints; i++) // Initialize permutation with identity
        perm[i] = i;                      // Set perm[i] = i
    for (int take = 0; take < clusterCount; take++) { // For each index to draw
        int j = take + rand() % (totalPoints - take); // Pick random position in [take..end)
        int tmp = perm[take];             // Save current value at 'take'
        perm[take] = perm[j];             // Move random item into 'take'
        perm[j] = tmp;                    // Put saved item at position j
        outIndices[take] = perm[take];    // Output the chosen index
    }
    free(perm);                           // Free permutation array
}

static int assign_points(const struct DataPoint *centroids, int clusterCount, int *pointCluster) {
    int changed = 0;                      // Counter for how many assignments changed this pass
    for (int i = 0; i < numDataPoints; i++) { // Loop over all points
        int best = 0;                     // Start assuming centroid 0 is closest
        float dx = dataPoints[i].x - centroids[0].x; // Delta X to centroid 0
        float dy = dataPoints[i].y - centroids[0].y; // Delta Y to centroid 0
        float bestDist2 = dx*dx + dy*dy;  // Squared distance to centroid 0
        for (int c = 1; c < clusterCount; c++) { // Check other centroids
            dx = dataPoints[i].x - centroids[c].x; // Delta X to centroid c
            dy = dataPoints[i].y - centroids[c].y; // Delta Y to centroid c
            float d2 = dx*dx + dy*dy;     // Squared distance to centroid c
            if (d2 < bestDist2) {         // If this centroid is closer
                bestDist2 = d2;           // Update best distance
                best = c;                 // Remember centroid index
            }
        }
        if (pointCluster[i] != best) {    // If assignment changed from previous iteration
            pointCluster[i] = best;       // Update assignment to new cluster
            changed++;                    // Count the change
        }
    }
    return changed;                       // Return number of points that switched clusters
}

static void recompute_centroids(struct DataPoint *centroids, int clusterCount, const int *pointCluster) {
    ensure_scratch(clusterCount);         // Make sure scratch buffers have size >= clusterCount
    memset(sumX_scratch, 0, (size_t)clusterCount * sizeof *sumX_scratch); // Zero X accumulators
    memset(sumY_scratch, 0, (size_t)clusterCount * sizeof *sumY_scratch); // Zero Y accumulators
    memset(count_scratch, 0, (size_t)clusterCount * sizeof *count_scratch); // Zero count accumulators
    for (int i = 0; i < numDataPoints; i++) { // Accumulate sums for each assigned cluster
        int c = pointCluster[i];          // Cluster id for point i
        sumX_scratch[c] += dataPoints[i].x; // Add point x to cluster sum
        sumY_scratch[c] += dataPoints[i].y; // Add point y to cluster sum
        count_scratch[c] += 1;            // Increment cluster count
    }
    for (int c = 0; c < clusterCount; c++) { // For each centroid
        if (count_scratch[c] > 0) {       // If cluster has at least one point
            centroids[c].x = sumX_scratch[c] / (float)count_scratch[c]; // Set centroid x to mean
            centroids[c].y = sumY_scratch[c] / (float)count_scratch[c]; // Set centroid y to mean
        }                                  // If empty, leave centroid as-is (simple policy)
    }
}

// ----------------------------------
// Output writer using bucket sorting
// ----------------------------------

static void write_output(const char *outputPath, const int *pointCluster) { // Writes sorted "x y cluster"
    int maxCluster = 0;                  // Track maximum cluster id seen
    for (int i = 0; i < numDataPoints; i++) // Scan assignments once
        if (pointCluster[i] > maxCluster)   // Update max if needed
            maxCluster = pointCluster[i];   // Record new max
    int clusterCount = maxCluster + 1;   // Compute number of clusters from max id

    int *counts = (int *)calloc((size_t)clusterCount, sizeof *counts); // Allocate counts per cluster, zeroed
    if (!counts) {                       // Check allocation
        perror("calloc");                // Report error
        exit(1);                         // Abort
    }
    for (int i = 0; i < numDataPoints; i++) // Count members per cluster
        counts[pointCluster[i]]++;       // Increment count for assigned cluster

    int *offset = (int *)malloc((size_t)clusterCount * sizeof *offset); // Allocate prefix offsets
    if (!offset) {                       // Check allocation
        perror("malloc");                // Report error
        free(counts);                    // Free counts before aborting
        exit(1);                         // Abort
    }
    offset[0] = 0;                       // First cluster starts at position 0
    for (int c = 1; c < clusterCount; c++) // Build prefix sums
        offset[c] = offset[c - 1] + counts[c - 1]; // Offset c = sum of previous counts

    int *order = (int *)malloc((size_t)numDataPoints * sizeof *order); // Array to store indices in bucketed order
    if (!order) {                        // Check allocation
        perror("malloc");                // Report error
        free(offset);                    // Free offset buffer
        free(counts);                    // Free counts buffer
        exit(1);                         // Abort
    }
    int *cursor = (int *)malloc((size_t)clusterCount * sizeof *cursor); // Working cursor per cluster
    if (!cursor) {                       // Check allocation
        perror("malloc");                // Report error
        free(order);                     // Free order buffer
        free(offset);                    // Free offset buffer
        free(counts);                    // Free counts buffer
        exit(1);                         // Abort
    }
    for (int c = 0; c < clusterCount; c++) // Initialize cursor = offset (start positions)
        cursor[c] = offset[c];           // Set cursor for cluster c

    for (int i = 0; i < numDataPoints; i++) { // Single pass to place each index into its bucket
        int c = pointCluster[i];         // Cluster id for point i
        int pos = cursor[c]++;           // Take current cursor position and advance
        order[pos] = i;                  // Place original index i at computed position
    }

    FILE *f = fopen(outputPath, "w");    // Open output file for writing
    if (!f) {                            // Check open success
        perror("fopen");                 // Report OS error
        free(cursor);                    // Free cursor buffer
        free(order);                     // Free order buffer
        free(offset);                    // Free offset buffer
        free(counts);                    // Free counts buffer
        exit(1);                         // Abort
    }
    for (int p = 0; p < numDataPoints; p++) { // Emit rows in bucketed order
        int i = order[p];                // Get original point index from order
        fprintf(f, "%.6f %.6f %d\n",     // Write "x y cluster" with fixed precision
                (double)dataPoints[i].x, // Cast float to double for printf
                (double)dataPoints[i].y, // Cast float to double for printf
                pointCluster[i]);        // Emit cluster id
    }
    fclose(f);                           // Close the output file

    free(cursor);                        // Free cursor buffer
    free(order);                         // Free order buffer
    free(offset);                        // Free offsets buffer
    free(counts);                        // Free counts buffer
}

// ---------------------------
// Public K-means entry point
// ---------------------------

void kmeansImplementation(int clusterCount) { // Runs K-means with given k
    if (clusterCount <= 0 || clusterCount > numDataPoints) { // Validate k against data size
        fprintf(stderr, "Error: k must be in [1, %d]\n", numDataPoints); // Inform about invalid k
        return;                        // Abort clustering early
    }
    static int seeded = 0;             // Track whether RNG has been seeded
    if (!seeded) {                     // If not yet seeded
        srand((unsigned)time(NULL));   // Seed RNG with current time
        seeded = 1;                    // Mark seeded
    }

    int *seedIdx = (int *)malloc((size_t)clusterCount * sizeof *seedIdx); // Array for initial centroid indices
    struct DataPoint *centroids =
        (struct DataPoint *)malloc((size_t)clusterCount * sizeof *centroids); // Centroid positions
    int *pointCluster = (int *)malloc((size_t)numDataPoints * sizeof *pointCluster); // Assignment per point
    if (!seedIdx || !centroids || !pointCluster) { // Verify allocations
        perror("malloc");               // Report allocation failure
        free(seedIdx);                  // Free any partial allocations
        free(centroids);                // Free centroids if allocated
        free(pointCluster);             // Free assignments if allocated
        return;                         // Abort (graceful)
    }

    pick_k_random_distinct_indices(numDataPoints, clusterCount, seedIdx); // Choose k distinct seeds
    for (int c = 0; c < clusterCount; c++) // Initialize centroids from chosen seeds
        centroids[c] = dataPoints[seedIdx[c]]; // Copy seed point into centroid array
    for (int i = 0; i < numDataPoints; i++) // Initialize all assignments to sentinel
        pointCluster[i] = -1;           // Use -1 so first assign_points counts as all-changed

    const int MAX_ITERS = 300;          // Upper bound on iterations to avoid infinite loops
    for (int it = 0; it < MAX_ITERS; it++) { // Iterative K-means loop
        int changed = assign_points(centroids, clusterCount, pointCluster); // Assign every point to nearest centroid
        recompute_centroids(centroids, clusterCount, pointCluster); // Move centroids to mean of assigned points
        if (changed == 0)               // If no point changed its cluster
            break;                      // We’re converged; stop iterating
    }

    write_output("kmeans-output.txt", pointCluster); // Emit results sorted by cluster using bucket sort

    free(seedIdx);                      // Free seed indices buffer
    free(pointCluster);                 // Free assignments buffer
    free(centroids);                    // Free centroid array
    free_scratch();                     // Free scratch buffers to keep valgrind happy
}
