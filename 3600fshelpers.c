#include "3600fshelpers.h"

/*
 * Find a dirent with the given path.
 * If found, fill in the given de with that dirent, and return 0
 * Otherwise return -ENOENT
 */
int find_dirent_by_name(dirent* de, const char* path, vcb* disk_vcb) {
  
  // Loop through dirents, looking for the valid dirent with a matching name
  for (int i = 0; i < disk_vcb->de_length; i++) {
    dirent* tmp = NULL;
    get_dirent(i, tmp, disk_vcb); 
    if ((tmp->valid == 1) && (strcmp(path, tmp->name) == 0)) {
       de = tmp;
       return 0;
    }
  }

  fprintf(stderr, "Could not find specified file: %s", path); 
  return -ENOENT;
}

/*
 * This function searches an array of fatents to find the next available one,
 * it then zeroes out the memory in the corresponding data block and returns
 * the corresponding index into the array of fatents.
 */
int get_new_fatent(fatent** fatents, vcb* disk_vcb) {
  int i;
  int found = 0;
  for (i = 0; i < disk_vcb->fat_length; i++) {
    if (!fatents[i]->used) {
      found = 1; 
      break;
    }
  }

  if (!found) return -1;

  fatents[i]->used = 1;
  fatents[i]->eof = 1;

  // Zero out the data
  char* zero_buff = calloc(BLOCKSIZE, 1);
  dwrite(disk_vcb->db_start + i, zero_buff);
  free(zero_buff);

  return i;
}

/* 
 * This function takes an index into the FAT and an offset and gets the FAT block
 * that is that many bytes away, creating new blocks along the way if it has to
 */
int get_fatent_from_offset(int start_index, int offset, fatent** fatents, vcb* disk_vcb) {
  fatent* start_block = fatents[start_index];
  // the offset / BLOCKSIZE will give us the number of blocks to move forward
  for (int i = offset / BLOCKSIZE; i > 0; i--) {
    if (!start_block->eof) {
      start_index = start_block->next;
      start_block = fatents[start_index];
    } else {
      // Create a new FAT Block if there isn't one to move forward to
      start_index = get_new_fatent(fatents, disk_vcb);
      if (start_index == -1) return -ENOSPC; // No more space
      
      start_block->next = start_index;
      start_block->eof = 0;
      start_block = fatents[start_index];
    }
  }
  return start_index;
}

/*
 * This function takes a block number and returns the data contents of that block
 * it then stores that block in a cache from which it will return the data if it
 * is requested again.
 */
void get_block(int index, char* buf) {
  // TODO: Implement caching.
  /* get_cached_block(index, buf);
   * if (NULL == buf) {
   */
   if(dread(index, buf) < 0) { fprinf(stderr, "dread failed for block %i\n", index); }
}

/* This function takes an index for a directory entry and finds that directory entry
 * on disk. 
 */
void get_dirent(int index, dirent* de, vcb* disk_vcb) {
  // Get the disk number where we will find the directory entry,
  // note that there are 4 dirents per block
  int block_index = disk_vcb->de_start + index / 4;

  // Get the block data
  char block_buffer[BLOCKSIZE];
  get_block(block_index, block_buffer);

  memcpy(de, block_buffer + (index % 4), sizeof(dirent));
}
