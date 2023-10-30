#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    FILE *fp;
    int i;

    fp = fopen("log", "a+");
    if(fp == NULL) {
        printf("file can't be opened\n");
        exit(1);
    }

    for (size_t i = 0; i < argc; i++) {
        fprintf(fp, "Mandos cleanup: arg %ld %s\n", i, argv[i]);
    }

    // The `valid` argument which is valid if 1 or 0 if the
    // verifier determined an invalid file
    return (argv[4][0] == '1') ? 0 : -1;
}
