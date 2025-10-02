// main.c
#include <stdio.h>
#include <stdlib.h>

void readData(char *filename);      // from kmeans.c (your version is fine)
void kmeansImplementation(int k);   // core loop (random init inside)
void freeData(void);                // frees your globals

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data-file-name> <k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load data (your readData currently opens "kmeans-data.txt" internally;
    // passing argv[1] is still fine even if it's ignored).
    readData(argv[1]);

    // Parse k (range check happens inside kmeansImplementation)
    int k = atoi(argv[2]);
    if (k <= 0) {
        fprintf(stderr, "Error: k must be > 0\n");
        freeData();
        return EXIT_FAILURE;
    }

    // Run K-means (centroids chosen randomly inside; positions & assignments evolve)
    kmeansImplementation(k);

    freeData();
    return EXIT_SUCCESS;
}
