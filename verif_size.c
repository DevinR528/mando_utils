#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char const *argv[]) {
    int err;
    struct stat buf;

    err = stat(argv[1], &buf);
    if (err != 0) {
        printf("file can't be opened\n");
        exit(1);
    }

    return (buf.st_size < 25) ? 0 : -1;
}
