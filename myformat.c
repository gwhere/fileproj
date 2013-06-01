#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mydisk.h"

struct superblock_s
{
  int p_size;  // in blocks
  int bm_loc;
  int root_loc;
  int data_loc;
};

typedef struct superblock_s *superblock;

superblock create_sb(disk_t disk, int size, int bm, int root, int data)
{
  superblock sb = malloc(disk->block_size);
  sb->p_size = size;
  sb->bm_loc = bm;
  sb->root_loc = root;
  sb->data_loc = data;
  return sb;
}

void main(int argc, char *argv[])
{
  char *disk_name;
  disk_t disk;
  unsigned char *databuf;
  int i, j;
  superblock sb;

  // Read the parameters
  if(argc != 2) {
    printf("Usage: testdisk <disk_name>\n");
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
  sb = create_sb(disk, 20, 1, 2, 19);
  writeblock(disk, 0, (char *)sb);
  free(sb); 

  // Read from the superblock
  sb = malloc(disk->block_size);
  readblock(disk, 0, (char *)sb);

  // Test superblock
  printf("sb->p_size = %d\n", sb->p_size);
  printf("sb->data_loc = %d\n", sb->data_loc);
/**************************************/

  /*
  // Write some blocks
  printf("Writing some blocks...");
  for(i = 0; i < disk->size; i++) {

    // Put some data in the block
    for(j = 0; j < BLOCK_SIZE; j++) 
      databuf[j] = i;

    printf("%d ", i);
    writeblock(disk, i, databuf);
  }

  printf("\nWrote successfully\n");

  // Read some blocks
  printf("Reading some blocks...");
  for(i = 0; i < disk->size; i++) {
    printf("%d ", i);
    readblock(disk, i, databuf);

    // Check the data in the block
    for(j = 0; j < BLOCK_SIZE; j++)
      if(databuf[j] != i) {
	printf("Main: read data different from written data!\n");
	exit(-1);
      }
  }

  printf("\nRead Successfully and data verified\n");

  // Seek back and read them again
  printf("Seeking back to block 0\n");
  seekblock(disk, 0);

  // Read some blocks
  printf("Reading some blocks...");
  for(i = 0; i < disk->size; i++) {
    printf("%d ", i);
    readblock(disk, i, databuf);

    // Check the data in the block
    for(j = 0; j < BLOCK_SIZE; j++)
      if(databuf[j] != i) {
	printf("Main: read data different from written data!\n");
	exit(-1);
      }
  }

  printf("\nRead Successfully and data verified\n");
  printf("Done reading and writing.\n");

  // Try seeking past the end of the disk
  printf("Seeking off the end of the disk\n");
  seekblock(disk, disk->size);
  */
}

