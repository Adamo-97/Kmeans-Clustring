#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double (*dataMatrix)[2] = NULL;
int numDataPoints = 0;          

void readData(char* filename) {
    FILE* dataFile = fopen(filename, "r");
    if (!dataFile) {
        printf("Failed to open %s\n", filename);
        exit(1);
    }

    double x, y;
    while (fscanf(dataFile, "%lf %lf", &x, &y) == 2) {
        // grow array by 1 row
        double (*tmp)[2] = realloc(dataMatrix, (numDataPoints + 1) * sizeof *dataMatrix);
        if (!tmp) { 
            perror("realloc"); 
            break; 
        }
        dataMatrix = tmp;

        // store values
        dataMatrix[numDataPoints][0] = x;
        dataMatrix[numDataPoints][1] = y;

        printf("%lf %lf\n", dataMatrix[numDataPoints][0], dataMatrix[numDataPoints][1]);

        numDataPoints++;
    }

    fclose(dataFile);
    printf("Total data points read: %d\n", numDataPoints);
}

void kmeansImplementation(int k) {
    int kook[k];
    int count = 0;
    srand(time(NULL));

    while (count < k) {
        int r = rand() % numDataPoints;

        int dup = 0;
        for (int j = 0; j < count; j++) {
            if (r == kook[j]) {
                dup = 1;
                break;
            }
        }

        if (!dup) {
            kook[count] = r;
            printf("centroid %d: %d\n", count, r);
            count++;
        }
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
