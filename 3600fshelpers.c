#include "3600fshelpers.h"

/*
 * Find a dirent with the given path.
 * If found, return a pointer to that dirent
 * Otherwise return a NULL pointer
 */

dirent* find_dirent(dirent** dirents, const char* path, int de_length) {
  int found = 0;
  int i;

  for (i = 0; i < de_length; i++) {
    if ((dirents[i]->valid == 1) && (strcmp(path, dirents[i]->name) == 0)) {
       found = 1;
       break;
    }
  }

  if (found) {
    return dirents[i];
  } else {
    fprintf(stderr, "Could not find specificied file\n");
    return NULL;
  }
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
