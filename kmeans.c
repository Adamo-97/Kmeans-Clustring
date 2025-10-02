#include <stdio.h>
#include <stdlib.h>

// Declaration of the DataPoint struct
struct DataPoint{
    float x;
    float y;
};

static struct DataPoint* dataPoints = NULL;
static int numDataPoints = 0;


void readData(char* filename) {
    FILE* dataFile = fopen("kmeans-data.txt", "r");
    if (!dataFile) {
        printf("Failed to open kmeans-data.txt");
        exit(0);
    }

    float x, y;
    while (fscanf(dataFile, "%f %f", &x, &y) == 2) {
        struct DataPoint* dp = realloc(dataPoints, (numDataPoints + 1) * sizeof(struct DataPoint)); // grow array by 1
    
        if(!dp) {perror("realloc"); break;}
        dataPoints = dp;

        dataPoints[numDataPoints].x = x;
        dataPoints[numDataPoints].y=y;
        numDataPoints++;
    }

    fclose(dataFile);
    printf("Total data points read: %d\n", numDataPoints);
}

void kmeansImplementation(int k) {

    for (int i = 0; i < k; i++) {
        int r = rand() & 1798;
        printf("current centroid: %d", r);
    }

    
}


// function to pick k random centroids or k by user input

// function that counts assign points, here we will compute distances using euclidean distance and pick the nearest centroid

// function that recomputes the centroids as the mean of assigned points

// kmeans main function that calls the above functions iteratively until convergence

// notes to keep in mind:
// 1. handle edge cases like k > n or k <= 0
// 2. one cluster with no points assigned, we can reinitialize it randomly
// 3. convergence criteria, max iterations or centroids not changing significantly
// 4. performance considerations for large datasets, maybe use kd-trees or ball-trees for nearest neighbor search


void freeData(void){
    free(dataPoints);
    dataPoints = NULL;
    numDataPoints =0;
}