#include "mydisk.h"

/*
 * Struct for superblock
 */

struct superblock_s
{
  int p_size;  // in blocks
  int root_loc;
  int max_files;
  int bm_loc;
  int map_blocks;
  int data_loc;
};

typedef struct superblock_s *superblock;


struct myfile_s {
  char filename[16];
  int f_size; //in bytes
  short block_list[64];
};

typedef struct myfile_s * myfile;
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
 * Adds an inode number to root dir
 */

void add_to_root(disk_t disk, superblock sb, int inode_num);



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
 * inode, or -1 if none are found. If mark>0, then
 * the found block is marked as used.
 */

int find_inode_space(disk_t disk, superblock sb, int mark);


/*
 * Creates an inode struct and returns it. Initialized
 * struct begins with no size or data.
 * Pre: filename is 
 */

myfile create_inode(disk_t disk, superblock sb, char * filename, \
		    int filename_length);



/*
 * This function writes an inode to disk. It returns the block
 * number of the inode just written
 */

int write_inode(disk_t disk, superblock sb, myfile f);



/*
 * Data is stored in a stack growing from the right.
 * This function works in basically the same way
 * as the find_inode_space function.
 */

int find_data_space(disk_t disk, superblock sb, int mark);





/*
 * Write file, handle all bytemap, inodes, and data mapping
 */

