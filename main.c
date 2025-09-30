#include <stdio.h>
#include <stdlib.h>
/* Function implemented in kmeans.c */
void greet_from_kmeans(void);

int main(void) {

    // here we will have to read the 2d data points from a file

    // parameter intake for k prompt he user or seed a random nr 1 < k < n


    int* nr = malloc(sizeof(int));
    printf("Hello from main.c 👋\n");
    greet_from_kmeans();
    return 0;
}
