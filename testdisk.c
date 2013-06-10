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

  // Open up the disk given in arguments
  disk_name = argv[1];
  disk = opendisk(disk_name);
  superblock sb = malloc(disk->block_size);
  readblock(disk, 0, (char *)sb);

  // Part 0 of testing
  //    Print initial files
  //    on disk
  printf("%s: \n", argv[0]);
  print_bitm(disk, sb);
  print_root(disk, sb);
  printf("\n");

  // Part 1 of testing
  //    Write file (if any) from the unix 
  //    directory onto the disk
  //    or delete if present
  if (argc > 2) {
    if (file_exists(disk, sb, argv[2])) {
      printf("File %s exists. File will be removed.\n", argv[2]);
      delete_file(disk, sb, argv[2]);
    } else {
      printf("Copy file %s to %s.\n", argv[2], argv[1]);
      copy_dir (disk, sb, argv[2]);
    }
  }

  // Part 2 of testing
  //    Print files written
  //    to disk
  if (argc > 2) {
    printf("%s: \n", argv[0]);
    print_bitm(disk, sb);
    print_root(disk, sb);
    printf("\n");
  }

  // Part 3 of testing
  //    Prints contents of file from 
  //    disk to stdout
  //    or shows it was deleted
  if (argc > 2) {
    if (file_exists(disk, sb, argv[2])) {
      printf("%s: Contents of %s from disk looks like this:\n",
           argv[0], argv[2]);
      print_file(disk, sb, argv[2]);
    } else {
      printf("File %s does not exist.\n", argv[2]);
    }
  }

  return 0;
}
