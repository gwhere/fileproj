#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "filesys.h"



void main(int argc, char *argv[])
{
  char *disk_name;
  disk_t disk;
  unsigned char *databuf;
  int i, j;
  superblock sb;

  // Read the parameters
  if(argc != 2) {
    printf("Usage: myformat <disk_name>\n");
    exit(-1);
  }

  disk_name = (char *)argv[1];

  // Open the disk
  disk = opendisk(disk_name);

  // Set up a buffer for writing and reading
  databuf = malloc(disk->block_size);

/********* superblock *****************/
// Cast sb as char* for read/write so mydisk doesn't complain

  // Create a new superblock for testing
  sb = create_sb(disk, 20, 1, 2);
  writeblock(disk, 0, (char *)sb);
  free(sb); 

  // Read from the superblock
  sb = malloc(disk->block_size);
  readblock(disk, 0, (char *)sb);

  // Test superblock
  print_sb(sb);



  //Create root directory
   create_root(disk, sb);

  //Print root directory
    print_root(disk, sb);

  //Initialize bitmap
  create_bm(disk, sb);

  //Print bitmap
  print_bm(disk, sb);

  //DEBUG
  printf("Next inode space = %d\n", find_inode_space(disk, sb,0));
  printf("Next data space = %d\n", find_data_space(disk, sb,0));
  add_to_root(disk, sb, "abc", 23);
  add_to_root(disk, sb, "def", 13);
  add_to_root(disk, sb, "thizz", 12);
  
  print_root(disk, sb);

}

