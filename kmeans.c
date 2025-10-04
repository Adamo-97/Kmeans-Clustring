#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// dynamic 2D array for data points "all of them will be stored here"
double (*dataMatrix)[2];
// number of data points read from file "will help us with edge cases and loops"
int numDataPoints;          


/*
    Function to ask the user if they want to assign initial centroid positions
    - if yes, it will read the positions into an array and return it
    - if no, it will return NULL
    - the function will allocate memory for the array if needed, and the caller is responsible for freeing it
    - the function will handle memory allocation errors
    - no input should be empty for a yes answer
    - the function will validate that the input positions are within the range of data points
    - if invalid input is detected, it will print an error message and exit the program
*/
int* askUserForIndexes(int numClusters) {
    int *userIndexes = NULL;
    char answer;
    int ch;  

    if (numClusters <= 0 || numDataPoints <= 0) {goto FAIL;}

    while (1) {
        printf("Do you want to choose initial centroid positions? (y/n): ");
        if (scanf(" %c", &answer) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF) {} // clear junk
            continue;
        }
        if (answer=='y' || answer=='Y' || answer=='n' || answer=='N') break;
        fprintf(stderr, "Please answer 'y' or 'n'.\n");
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    if (answer=='n' || answer=='N') {
        return NULL;
    }

    userIndexes = (int*)malloc((size_t)numClusters * sizeof *userIndexes);
    if (!userIndexes) goto FAIL;

    int i = 0;
    while (i < numClusters) {
        int indexValue;
        printf("Enter centroid index %d of %d (0..%d): ",
               i + 1, numClusters, numDataPoints - 1);

        if (scanf("%d", &indexValue) != 1) {
            fprintf(stderr, "Not an integer. Try again.\n");
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue; 
        }
        if (indexValue < 0 || indexValue >= numDataPoints) {
            fprintf(stderr, "Out of range (valid 0..%d). Try again.\n", numDataPoints - 1);
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue; 
        }

        userIndexes[i++] = indexValue;    
        while ((ch = getchar()) != '\n' && ch != EOF) {} 
    }

    return userIndexes;

FAIL:
    if (userIndexes) free(userIndexes);
    fprintf(stderr, "askUserCentroidIndices: input/allocation failed — exiting.\n");
    exit(1);
}

/*
    Function will clean up allocated memory for data points
*/
void cleanup() {
    free(dataMatrix);
    dataMatrix = NULL;
    numDataPoints = 0;
}

/* Function to read data points from a file
    - the file is expected to have two columns of floating point numbers representing x and y coordinates
    - the function will store them in a dynamically allocated 2D array "dataMatrix"
    - it will also update the global variable numDataPoints to reflect the number of points read after resetting them
    - it will handle memory allocation and resizing as needed, 64 initially allocated then doubled as needed
    - prints each data point as it is read and the total number of points read at the end "will be commented out later but useful for debugging"
    - handles file opening errors and memory allocation errors
    - cleans up allocated memory in case of errors
*/
void readData(char* filename) {
    FILE* dataFile = fopen(filename, "r");
    if (!dataFile) goto FAIL;
    // printf("[FILE OPENED SUCCESSFULLY !]\n");
    double x, y;
    int capacity = 64;

    numDataPoints = 0;
    free(dataMatrix);
    dataMatrix = NULL;

    dataMatrix = malloc(capacity * sizeof *dataMatrix);
    if (!dataMatrix) goto FAIL;

    while (fscanf(dataFile, "%lf %lf", &x, &y) == 2) {
        if (numDataPoints >= capacity) {
            capacity *= 2;
            double (*tmp)[2] = realloc(dataMatrix, capacity * sizeof *dataMatrix);
            if (!tmp) goto FAIL;
            dataMatrix = tmp;
        }

        dataMatrix[numDataPoints][0] = x;
        dataMatrix[numDataPoints][1] = y;

        // printf("%lf %lf\n", dataMatrix[numDataPoints][0], dataMatrix[numDataPoints][1]);
        numDataPoints++;
    }

    fclose(dataFile);
    // printf("Total data points read: %d\n", numDataPoints);
    return;

FAIL:
    if (dataFile) fclose(dataFile);
    if (dataMatrix) cleanup();
    fprintf(stderr, "Error reading data file (check for a typo in the filename): %s\n", filename);
    exit(1);
}

/* Function to initialize centroids for K-means clustering
    - takes the number of clusters (numClusters) and an array to store the indexes of chosen centroids "indexes are either a userinput or randomly chosen"
    - if the array is NULL, it will randomly select unique centroidIndexes from the data points
    - checks for edge cases where numClusters is greater than the number of data points or less than or equal to zero
    - if invalid, prints an error message and exits the program after cleaning up allocated memory
*/
int* initializeCentroids(int numClusters, int* centroidsIndexes) {

    int* finalCentroids = NULL;
    if (centroidsIndexes == NULL) {
        int count = 0;
        centroidsIndexes = (int*)malloc(numClusters * sizeof *centroidsIndexes);

        if (!centroidsIndexes) goto FAIL;

        srand(time(NULL));
        while (count < numClusters) {
            int randomIndex = rand() % numDataPoints;
            int duplicate = 0;
            for (int j = 0; j < count; j++) {
                if (randomIndex == centroidsIndexes[j]) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                centroidsIndexes[count] = randomIndex;
                count++;
                finalCentroids = centroidsIndexes;
            }
        }
        return finalCentroids;
    }

    finalCentroids = (int*)malloc(numClusters * sizeof *finalCentroids);
    if (!finalCentroids) goto FAIL;
    for (int i = 0; i < numClusters; i++) {
        finalCentroids[i] = centroidsIndexes[i];
    }
    return finalCentroids;

FAIL:
    fprintf(stderr, "invalid number of clusters: %d\n", numClusters);
    fprintf(stderr, "it should be > 0 and <= number of data points: %d\n", numDataPoints);
    if(dataMatrix) cleanup();
    exit(1);
}
/*
    Function to compute Euclidean distance between two data points by index
    - uses the global dataMatrix[numDataPoints][2] (x,y)
    - validates preconditions (matrix allocated, indices in range)
    - returns the non-negative distance as a double
    - on invalid input/state, prints an error, cleans up, and exits(1)
*/
double euclideanDistanceByIndex(int indexFirst, int indexSecond) {
    if (!dataMatrix || numDataPoints <= 0) {
        fprintf(stderr, "euclideanDistanceByIndex: dataMatrix is not initialized.\n");
        goto FAIL;
    }
    if (indexFirst < 0 || indexFirst >= numDataPoints ||
        indexSecond < 0 || indexSecond >= numDataPoints) {
        fprintf(stderr,
                "euclideanDistanceByIndex: index out of range (got %d, %d; valid 0..%d).\n",
                indexFirst, indexSecond, numDataPoints - 1);
        goto FAIL;
    }

    double deltaX = dataMatrix[indexFirst][0] - dataMatrix[indexSecond][0];
    double deltaY = dataMatrix[indexFirst][1] - dataMatrix[indexSecond][1];
    return sqrt(deltaX * deltaX + deltaY * deltaY);

FAIL:
    if (dataMatrix) cleanup();
    exit(1);
}

/* K-means clustering implementation */
void kmeansImplementation(char* dataFileName, int numClusters) {

    int* finalCentroids = NULL;
    
    readData(dataFileName);
    if(numClusters <= 0 || numClusters > numDataPoints) goto FAIL;

    finalCentroids = initializeCentroids(numClusters, askUserForIndexes(numClusters));

    // For demonstration delete later
    printf("Initial centroid positions (indexes):\n");
    for (int i = 0; i < numClusters; i++) {
        printf("Centroid %d: Index %d -> (%.2lf, %.2lf)\n", 
               i, finalCentroids[i], 
               dataMatrix[finalCentroids[i]][0], 
               dataMatrix[finalCentroids[i]][1]);
    }
    printf("\nEuclidean distances between centroids:\n");
    for (int i = 0; i < numClusters; i++) {
        for (int j = i + 1; j < numClusters; j++) {
            double dist = euclideanDistanceByIndex(finalCentroids[i], finalCentroids[j]);
            printf("Distance between Centroid %d and Centroid %d: %.4lf\n", 
                   i, j, dist);
        }
    }
    printf("\nTotal data points: %d\n", numDataPoints);
    printf("Total clusters: %d\n", numClusters);
    printf("Centroid indexes: ");
    for (int i = 0; i < numClusters; i++) {
        printf("%d ", finalCentroids[i]);
    }
    printf("\n");
    printf("Centroid coordinates:\n");
    for (int i = 0; i < numClusters; i++) {
        printf("Centroid %d: (%.2lf, %.2lf)\n", 
               i, dataMatrix[finalCentroids[i]][0], dataMatrix[finalCentroids[i]][1]);
    }
    printf("\n");
    // end of demonstration

    // Free allocated memory and clean up "måste fillas på med resten av kmeans"
    if (finalCentroids) free(finalCentroids);
    if (dataMatrix) cleanup();
    return;
FAIL:
    fprintf(stderr, "KMEANS: nr of clusters is invalid\n");
    if (finalCentroids) free(finalCentroids);
    if (dataMatrix) cleanup();
    exit(1);
}
