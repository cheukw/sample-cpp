#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1000

int main() {
  char *path = calloc(BUF_SIZE, 1);
  strcat(path, getenv("HOME"));
  strcat(path, "/.bashrc");
  FILE *foo = fopen(path, "r");

  free(path);
  fclose(foo);
  return foo == NULL;
}