#include <stdio.h>

// Declaration of the DataPoint struct
struct DataPoint{
    double x;
    double y;
};

// function to pick k random centroids or k by user input

// function that counts assign points, here we will compute distances using euclidean distance and pick the nearest centroid

// function that recomputes the centroids as the mean of assigned points

// kmeans main function that calls the above functions iteratively until convergence

// notes to keep in mind:
// 1. handle edge cases like k > n or k <= 0
// 2. one cluster with no points assigned, we can reinitialize it randomly
// 3. convergence criteria, max iterations or centroids not changing significantly
// 4. performance considerations for large datasets, maybe use kd-trees or ball-trees for nearest neighbor search
/* Defined exactly once here */
void greet_from_kmeans(void) {
    printf("Hello from kmeans.c 🌟\n");
}
