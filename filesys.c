#include "filesys.h"
#include <string.h>


superblock create_sb(disk_t disk, int size, int root, int bm)
{
  superblock sb = malloc(disk->block_size);
  sb->p_size = size;

  sb->root_loc = root;
  //max # of files (misleading since each file requires at least 2 blocks (inode+data)
  int max_files = (disk->block_size / sizeof(dir_entry));
  // check if max_files exceeds free space after root
  if(max_files > sb->p_size) max_files = sb->p_size;  
  sb->max_files = max_files;

  sb->bm_loc = bm;
  //Calculate # of blocks needed for bitmap
  sb->map_blocks = (size/(disk->block_size * 8)) + 1;
  if ((size%(disk->block_size * 8)) == 0) sb->map_blocks -= 1;
  sb->data_loc = sb->map_blocks;

  return sb;
}



void print_sb(superblock sb) {
  printf("Contents of superblock:\n");
  printf("partition size: %d, root loc: %d, bitmap loc: %d, inode/data loc: %d\n", \
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
    if (root[i].inode_num > 0)
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

int search_root(disk_t disk, superblock sb, char * f_name) {

  char databuf[disk->block_size];
  readblock(disk, sb->root_loc, databuf);
  dir_entry root = (dir_entry) databuf;
  int i;

  for(i=0; i<sb->max_files; ++i) {
    if(strcmp(root[i].filename, f_name) == 0) {
      return root[i].inode_num;
    }  
  }
  return -1;
}


int getbit(int bit, unsigned char byte) {
  //printf("hex: %X ", (byte & (1 << bit)));
  return ((byte & (1 << bit)) > 0);
}

unsigned char modbit(int bit, unsigned char byte, int val)
{
  // change 0 to 1 if val == 1
  if (getbit(bit, byte) < (val > 0)) {
  	//printf("0->1: %X\n", ((val > 0) << bit));
  	return byte + (1 << bit);
  } 
  // change 1 to 0 if val == -1
  else if (getbit(bit, byte) > (val >= 0)) {
  	return byte - (1 << bit);
  } 
  // no change
  else {
  	return byte;
  }
}

void create_bitm(disk_t disk, superblock sb) {
  unsigned char bytemap[disk->block_size];

  //Fill in first block of bitmap
  int i, j, k, l;
  j = sb->bm_loc + 1;
  for(i = 0; i < disk->block_size; i++) bytemap[i] = 0;
  bytemap[0] = modbit(0, bytemap[0], 1); //for superblock
  bytemap[0] = modbit(sb->root_loc, bytemap[0], 1);
  bytemap[0] = modbit(sb->bm_loc, bytemap[0], 1);
  
  while (j < sb->map_blocks + sb->bm_loc && j < disk->block_size * 8) {
    bytemap[j / 8] = modbit(j % 8, bytemap[j / 8], 1);
    j++;
  }
  //printf("bm[0]: %X\n", modbit(sb->root_loc, bytemap[0], 1));
  writeblock(disk, sb->bm_loc, bytemap);

  //If necessary, write other bitmap blocks
  //this makes it so bm_loc needs to come last
  if(sb->map_blocks > 1) {
    for(k = 1; k < sb->map_blocks; k++) {
      //reset bitmap for next block
      for(i = 0; i < disk->block_size; i++) bytemap[i]=0;

      //If TRULY necessary for a sufficiently large fs,
      //extend bitmap into additional blocks 
      while (j < sb->map_blocks + sb->bm_loc && j < (disk->block_size * 8 * (k + 1))) {
        l = j % (disk->block_size * 8);
        bytemap[l / 8] = modbit(l % 8, bytemap[l / 8], 1);
        j++;
      }
      writeblock(disk, sb->bm_loc+i, bytemap);
    }
  }
}

// return # of bytes needed to represent 'bits' # of bits
int bits_to_bytes(int bits) {
  int bytes = (bits / 8) + 1;
  if (bytes % 8 == 0)
    bytes -= 1;
  return bytes;
}

void print_bitm(disk_t disk, superblock sb) {
  unsigned char databuf[disk->block_size];
  
  int i,j, k, bitcount;
  bitcount = sb->p_size;
  printf("The bitmap currently looks like this:\n");
  for(i=0; i < sb->map_blocks; i++) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where bitmap takes more than 1 block
    int bytes = bits_to_bytes(sb->p_size) - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j=0; j < bytes; j++) {
      //printf("%02X ", databuf[j]);
      for (k = 0; k < 8; k++) {
        printf("%d ", getbit(k, databuf[j]));
        bitcount--;
        if (bitcount == 0) {
          putchar('\n');
          return;
        }
      }
    }
    putchar('\n');
  }
}

int find_inode_space_bit(disk_t disk, superblock sb, int mark) {
  unsigned char databuf[disk->block_size];

  int i,j, k, bitcount;
  bitcount = sb->p_size;
  int current_block = 0;
  for(i=0; i < sb->map_blocks; i++) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where bitmap takes more than 1 block
    int bytes = bits_to_bytes(sb->p_size) - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j=0; j < bytes; j++) {
      for (k = 0; k < 8; k++) {
        if(getbit(k, databuf[j])==0) {
	  if(mark>0) {
	    databuf[j] = modbit(k, databuf[j], mark);
	    writeblock(disk, sb->bm_loc + i, databuf);
	  }
	  return current_block;
        }
        ++current_block;
        if (bitcount == current_block) {
          return -1;
        }
      }
    }
  }
  return -1;
}

int find_data_space_bit(disk_t disk, superblock sb, int mark) {
  unsigned char databuf[disk->block_size];

  int i,j, k, k0;
  int current_block = sb->p_size - 1;
  for(i= sb->map_blocks - 1; i >= 0; --i) {
    readblock(disk, sb->bm_loc + i, databuf);

    //Handles case where bitmap takes more than 1 block
    int bytes = bits_to_bytes(sb->p_size) - (i*disk->block_size);
    if(bytes >= disk->block_size) bytes = disk->block_size;

    for(j = bytes-1; j >= 0; --j) {
      k0 = 7;
      if (j == bytes-1) k0 = (current_block % (k0 + 1));
      for (k = k0; k >= 0; k--) {
        if(getbit(k, databuf[j])==0) {
	  if(mark>0) {
	    databuf[j] = modbit(k, databuf[j], mark);
	    writeblock(disk, sb->bm_loc + i, databuf);
	  }
	  return current_block;
        }
        --current_block;
        if (current_block < 0) {
          return -1;
        }
      }
    }
  }
  return -1;
}

void write_string_file(disk_t disk, superblock sb, char * filename,	\
		       char * data_string) {
 
  int i = 0;
  if(strlen(filename) > 16) {
    fprintf(stderr, "%s: Filename is too long.\n", filename);
    return;
  }
  if(file_exists(disk, sb, filename)) {
    fprintf(stderr, "%s: Filename already in use.\n", filename);
    return;
  }

  //set up directory entry and mark bitmap
  if (find_inode_space_bit(disk, sb, 0) < 0) {
    fprintf(stderr, "%s: No free space. File not copied.\n", filename, i);
    return;
  }
  int i_num = find_inode_space_bit(disk, sb, 1);
  add_to_root(disk, sb, filename, i_num);
  
  //create file
  myfile mf = malloc(disk->block_size);
  mf->f_size = strlen(data_string) / 512;
  if (strlen(data_string) % 512 != 0) mf->f_size += 1;
  //printf("file size: %d\n", strlen(data_string));
  int data_loc;

  for (i = 0; i < mf->f_size - 1; i++)
  {
  	data_loc = find_data_space_bit(disk, sb, 0);
        if (data_loc < 0) {
          writeblock(disk, i_num, (char *)mf);
          fprintf(stderr, "%s: File is too large. Only %d blocks copied.\n", filename, i);
          perror("write_string_file"); 
          free(mf);
          return;
        }
        data_loc = find_data_space_bit(disk, sb, 1);
  	mf->block_list[i] = data_loc;
  	writeblock(disk, data_loc, strndup(data_string + BLOCK_SIZE * i, BLOCK_SIZE));
  	//printf("file chunk: %d: %s\n", mf->block_list[i], strndup(data_string + BLOCK_SIZE * i, BLOCK_SIZE));
  }
  data_loc = find_data_space_bit(disk, sb, 0);
  if (data_loc < 0) { 
    writeblock(disk, i_num, (char *)mf);
    fprintf(stderr, "%s: File is too large. Only %d blocks copied.\n", filename, i);
    perror("write_string_file");
    free(mf);
    return;
  }
  data_loc = find_data_space_bit(disk, sb, 1);
  mf->block_list[i] = data_loc;
  writeblock(disk, i_num, (char *)mf);
  writeblock(disk, data_loc, data_string + BLOCK_SIZE * i);
  //printf("file chunk %d: %s\n", mf->block_list[i], data_string + BLOCK_SIZE * i);
  free(mf);
}

myfile open_file(disk_t disk, superblock sb, char * f_name) {

  int i_num = search_root(disk, sb, f_name);
  unsigned char *databuf = malloc(disk->block_size);
  readblock(disk, i_num, databuf);
  myfile mf = (myfile)databuf;
  return mf;

}

void print_file(disk_t disk, superblock sb, char * f_name) {

  if(!file_exists(disk, sb, f_name)) {
    fprintf(stderr, "%s: File does not exist.\n", f_name);
    //perror("print_file");
    return;
  }
  myfile mf = open_file(disk, sb, f_name);
  int i;
  unsigned char *databuf = malloc(disk->block_size * mf->f_size);
  for(i=0; i<MAX_F_SIZE; ++i) {
    if(mf->block_list[i]==0) {
      break;
    }
    readblock(disk, mf->block_list[i], databuf);
    printf("%s", databuf);
  }
  putchar('\n');
}

/* Some helpers */

int file_exists(disk_t disk, superblock sb, char * filename) {
  return (search_root(disk, sb, filename) >= 0);
}

void mark_block_in_bitm(disk_t disk, superblock sb, int mark, int block) {
  unsigned char databuf[disk->block_size];

  int i,j, k, bitcount;
  bitcount = sb->p_size;
  int current_block = 0;

  i = block / (disk->block_size * 8);
  j = (block / 8) - (disk->block_size * i);
  k = block % 8;

  readblock(disk, sb->bm_loc + i, databuf);

  if(mark!=0) {
    databuf[j] = modbit(k, databuf[j], mark);
    writeblock(disk, sb->bm_loc + i, databuf);
  }
}

/****** Extra Credit ******/

int delete_from_root(disk_t disk, superblock sb, char * f_name) {

  char databuf[disk->block_size];
  readblock(disk, sb->root_loc, databuf);
  dir_entry root = (dir_entry) databuf;
  int i, j, inum;

  for(i=0; i<sb->max_files; ++i) {
    if(strcmp(root[i].filename, f_name) == 0) {
      inum = root[i].inode_num;
      //null filename
      for(j=0; j < 16; j++) root[i].filename[j]='\0';
      //invalid inode;
      root[i].inode_num=0;
      writeblock(disk, sb->root_loc, (char *) root);
      return inum;
    }  
  }
  return -1;
}

void delete_file(disk_t disk, superblock sb, char * filename) {
  int inum, i;
  myfile mf;
  if (file_exists(disk, sb, filename)) {
    myfile mf = open_file(disk, sb, filename);
    inum = delete_from_root(disk, sb, filename);

    mark_block_in_bitm(disk, sb, -1, inum);
    for(i=0; i<MAX_F_SIZE; ++i) {
      if(mf->block_list[i]==0) {
        break;
      }
      mark_block_in_bitm(disk, sb, -1, mf->block_list[i]);
    }
  }
}

