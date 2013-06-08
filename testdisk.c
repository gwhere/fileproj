#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"
#include "filesys.h"

void copy_dir (disk_t disk, superblock sb, char *filename) {
  // Open file for reading
  FILE *infile = fopen(filename, "r");
  if (infile == NULL) {
    fprintf(stderr, "%s: bad filename.\n", filename);
  }
  // Determine size of file
  // add move pointer to beginning of file
  fseek(infile, 0, SEEK_END);
  long f_size = ftell(infile);
  fseek(infile, 0, SEEK_SET);
  // Copy contents to buffer and write to disk
  char *data = malloc(f_size);
  fread(data, 1, f_size, infile);
  write_string_file(disk, sb, filename, data);
}

int main (int argc, char **argv) {
  // Declare pointers for disk and superblock
  char *disk_name;
  disk_t disk;
  superblock sb = malloc(disk->block_size);

  // Open up the disk given in arguments
  disk_name = argv[1];
  disk = opendisk(disk_name);
  readblock(disk, 0, (char *)sb);

  // Part 1 of testing
  //    Write files (if any) from the unix 
  //    directory onto the disk
  copy_dir (disk, sb, "test.in");
  
  // Part 2 of testing
  //    Print contents of files written
  //    to disk
  printf("%s: \n", argv[0]);
  print_bm(disk, sb);
  print_root(disk, sb);

  // Part 3 of testing
  //    Prints contents of test.in from 
  //    disk to stdout
  printf("%s: Contents of test.in from disk looks like this:\n",
         argv[0]);
  print_file(disk, sb, "test.in");
  return 0;
}
