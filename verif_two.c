#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LEN = 10;
int main(int argc, char const *argv[]) {
  FILE *fd;
  char string[LEN];

  if (argc < 2) {
    return -1;
  }

  fd = fopen(argv[1], "r");

  if (!fgets(string, LEN, fd)) {
    printf("fgets failed\n");
    return -1;
  }

  printf("string = `%s`\n", string);

  return (strncmp(string, "yo 03", 5) == 0 || strncmp(string, "yo 12", 5) == 0)
    ? 0
    : -1;
}
