#include <stdio.h>
#include <stdlib.h>

void readData(char* filename);
void freeData();
void kmeansImplementation(int k);

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("Error! Usage: ./kmeans <data-file-name> <number-of-centroids>");
        exit(0);
    }

    int k = atoi(argv[2]);

    readData(argv[1]);

    kmeansImplementation(k);

    freeData();
    return 0;
}
