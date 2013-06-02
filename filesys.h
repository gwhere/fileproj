#include "mydisk.h"

/*
 * Struct for superblock
 */

struct superblock_s
{
  int p_size;  // in blocks
  int root_loc;
  int bm_loc;
  int map_blocks;
  int data_loc;
};

typedef struct superblock_s *superblock;


/*
 *Create superblock
 */

superblock create_sb(disk_t disk, int size, int root, int bm);


/*
 * Prints superblock data for debugging
 */
void print_sb(superblock sb);

/*
 * Creates root node
 */

void create_root(disk_t disk, superblock sb);

/*
 * Prints contents of root node
 */

void print_root(disk_t disk, superblock sb);

/*
 * Initializes bytemap, which is just an array
 * of inodes in our filesystem.
 */

void create_bm(disk_t disk, superblock sb);

/*                                                                                                                                                                     
 *Prints content of bytemap                                                                                                                                            
 */

void print_bm(disk_t disk, superblock sb);

/*
 * Inodes are stored in stack-like formation starting
 * from the left of inode/data partition, This function
 * returns the first 1-block space suitable for an
 * inode, or -1 if none are found
 */

int find_inode_space(disk_t disk, superblock sb);

/*
 * Data is stored in a stack growing from the right.
 * This function works in basically the same way
 * as the previous function.
 */

int find_data_space(disk_t disk, superblock sb);
