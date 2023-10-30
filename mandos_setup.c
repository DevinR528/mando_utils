#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    FILE *fp;
    char name[50];
    int roll_no,  i, n;
    float marks;

    fp = fopen("log", "a+");
    if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }

    for (size_t i = 0; i < argc; i++) {
        fprintf(fp, "Mandos setup: arg %ld %s\n", i, argv[i]);
    }

    return 0;
}
