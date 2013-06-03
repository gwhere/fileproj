#include "filesys.h"

superblock create_sb(disk_t disk, int size, int root, int bm)
{
  superblock sb = malloc(disk->block_size);
  sb->p_size = size;

  sb->root_loc = root;
  //max # of files
  int max_files = disk->block_size / sizeof(dir_entry); 
  if(max_files > sb->p_size) max_files = sb->p_size;  
  sb->max_files = max_files;

  sb->bm_loc = bm;
  //Calculate # of blocks needed for bytemap
  sb->map_blocks = ((size -2)/(disk->block_size))+1;
  sb->data_loc = bm + sb->map_blocks;

  return sb;
}



void print_sb(superblock sb) {
  printf("Contents of superblock:\n");
  printf("partition size: %d, root loc: %d, bytemap loc: %d, inode/data loc: %d\n", \
	 sb->p_size, sb->root_loc, sb->bm_loc, sb->data_loc); 
}



void create_root(disk_t disk, superblock sb) {
  
  dir_entry root = malloc(sizeof(struct dir_entry_s) * sb->max_files);
  int i, j;
  for(i=0; i < sb->max_files; ++i) {
    //null filename
    for(j=0; j < 16; j++) root[i].filename[j]='\0';
    //invalid inode;
    root[i].inode_num=0;
  }
  writeblock(disk, sb->root_loc, (char *) root);
  free(root);
}



void print_root(disk_t disk, superblock sb) {

  unsigned char databuf[disk->block_size];
  readblock(disk, sb->root_loc, databuf);
  int i;
  dir_entry root = (dir_entry) databuf;
  printf("Inodes in root directory:\n");
  for(i=0; i < sb->max_files; ++i) {
    printf("%d: %s\n", root[i].inode_num, root[i].filename);
  }
}



void add_to_root(disk_t disk, superblock sb, char * fname, int inode_num) {
    unsigned char databuf[disk->block_size];
    readblock(disk, sb->root_loc, databuf);
    dir_entry root = (dir_entry) databuf;
    int i;
    
    //find free spot to store inode #
    int free_spot=-1;
    for(i=0; i < sb->max_files; ++i) {
      if(root[i].inode_num == 0) {
	free_spot = i;
	break;
      }
    }
    
    if(free_spot == -1) {
      printf("Cannot add file to root, root directory is full.\n");
      perror("add_to_root");
    }


    //Update directory and write to disk.
    root[free_spot].inode_num = inode_num;
    //Copy null-terminated filename to dir_entry
    for(i=0; i <16; i++) {
      if(fname[i]!='\0') {
	root[free_spot].filename[i] = fname[i];
      }
      else break;
    }
    writeblock(disk, sb->root_loc, (char *) root);
}



void create_bm(disk_t disk, superblock sb) {
  unsigned char bytemap[disk->block_size];

  //Fill in first block of bytemap
  int i;
  for(i=0; i< disk->block_size; i++) bytemap[i]=0;
  bytemap[0] = 1; //for superblock
  bytemap[sb->root_loc] = 1;
  bytemap[sb->bm_loc] = 1;
  writeblock(disk, sb->bm_loc, bytemap);

  //If necessary, write other bytemap blocks
  if(sb->map_blocks > 1) {
    //remove 1's from buffer
    bytemap[sb->root_loc] = 0;
    bytemap[sb->bm_loc] = 0;
    for(i=0; i< sb->map_blocks - 1; i++) {
      writeblock(disk, sb->bm_loc+i, bytemap);
    }
  }
}



void print_bm(disk_t disk, superblock sb) {
  unsigned char databuf[disk->block_size];
  
  int i,j;
  printf("The bytemap currently looks like this:\n");
  for(i=0; i < sb->map_blocks; i++) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where less than a full block left in bytemap
    int bytes = sb->p_size - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j=0; j < bytes; j++) printf("%d ", databuf[j]);
    putchar('\n');
  }
}



int find_inode_space(disk_t disk, superblock sb, int mark) {
  unsigned char databuf[disk->block_size];

  int i,j;
  int current_block = 0;
  for(i=0; i < sb->map_blocks; i++) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where less than a full block left in bytemap
    int bytes = sb->p_size - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j=0; j < bytes; j++) {
      if(databuf[j]==0) {
	if(mark>0) databuf[j] = 1;
	return current_block;
      }
      ++current_block;
    }
  }
  return -1;
}




int find_data_space(disk_t disk, superblock sb, int mark) {
  unsigned char databuf[disk->block_size];

  int i,j;
  int current_block = sb->p_size - 1;
  for(i= sb->map_blocks - 1; i >= 0; --i) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where less than a full block left in bytemap
    int bytes = sb->p_size - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j = bytes-1; j >= 0; --j) {
      if(databuf[j]==0) {
	if(mark>0) databuf[j]=1;
	return current_block;
      }
      --current_block;
    }
  }
  return -1;
}


