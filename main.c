#include <stdio.h>
#include <stdlib.h>

void readData(char* filename);
void kmeansImplementation(int k);

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("Error! Usage: ./kmeans <data-file-name> <number-of-clusters>");
        exit(0);
    }

    // char choice;

    // printf("Would you like to assign the initial centroid positions? (Y/N)")
    // scanf("%c", choice);

    // if (choice == "Y" || choice == "y") {
    //     printf("")
    // }

    int k = atoi(argv[2]);

    readData(argv[1]);
    kmeansImplementation(k);
    return 0;
}
