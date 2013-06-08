#include "mydisk.h"

#define MAX_F_SIZE 64 //max file size in blocks

/*
 * Struct for superblock
 */

struct superblock_s
{
  int p_size;  // size of partition in blocks
  int root_loc; //location of root directory
  int max_files; //maximum files that could fit in root dir
  int bm_loc; //location of bytemap
  int map_blocks; //blocks the bytemap takes up
  int data_loc; //location of data/inode block

};

typedef struct superblock_s *superblock;




struct myfile_s {
  int f_size; //in bytes
  short block_list[MAX_F_SIZE];
};

typedef struct myfile_s * myfile;



struct dir_entry_s {
  char filename[16];
  short inode_num;
};

typedef struct dir_entry_s * dir_entry;


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

void add_to_root(disk_t disk, superblock sb, char * filename, int inode_num);



/*
 * Looks for a filename in root and returns the 
 * inode number if found, or -1 if not found.
 */

int search_root(disk_t disk, superblock sb, char * filename);



/*
 * Initializes bytemap
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
 * Writes 1 block file to disk, whose only data is 
 * a string given as input
 */

void write_string_file(disk_t disk, superblock sb, char * filename, \
		       char * data_string);

/*
 * Returns the file struct associated with a certain filename
 */

myfile open_file(disk_t disk, superblock sb, char * filename);

/*
 * Prints the contents of the file system
 */
void print_file(disk_t, superblock, char *);
