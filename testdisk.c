#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"
#include "filesys.h"
#define LINE_SIZE 84

void copy_dir (disk_t disk, superblock sb, char *filename) {
  // Open file for reading
  FILE *infile = fopen(filename, "r");
  if (infile == NULL) {
    fprintf(stderr, "%s: bad filename.\n", filename);
  }
  // Determine size of file
  fseek(infile, 0, SEEK_END);
  long f_size = ftell(infile);
  fseek(infile, 0, SEEK_SET);
  // Copy contents to buffer and write to disk
  char *line = malloc(LINE_SIZE);
  fread(line, 1, f_size, infile);
  write_string_file(disk, sb, filename, line);
}
