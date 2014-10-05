/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format 
 * your disk file).
 */

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "3600fs.h"
#include "disk.h"

// vcb structure
typedef struct vcb_s {
  // a magic number of identity of your disk
  int magic;

  // description of disk layout
  int blocksize;
  int de_start;
  int de_length;
  int fat_start;
  int fat_length;
  int db_start;

  // meta data for root dir
  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
} vcb;

typedef struct dirent_s {
  unsigned int valid;
  unsigned int first_block;
  unsigned int size;
  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
  char name[452]; // BLOCKSIZE - sizeof(rest of struct)
} dirent;

typedef struct fatent_s {
  unsigned int used:1;
  unsigned int eof:1;
  unsigned int next:30;
} fatent;

void myformat(int size) {
  // Do not touch or move this function
  dcreate_connect();

  /* 3600: FILL IN CODE HERE.  YOU SHOULD INITIALIZE ANY ON-DISK
           STRUCTURES TO THEIR INITIAL VALUE, AS YOU ARE FORMATTING
           A BLANK DISK.  YOUR DISK SHOULD BE size BLOCKS IN SIZE. */
  
  ////////////////////////
  // Initialize the VCB //
  ////////////////////////
  vcb myvcb;
  myvcb.blocksize = BLOCKSIZE;
  myvcb.de_start = 1;
  myvcb.de_length = ceil(size / 2); //TODO: Need to change this to be more efficient later
  myvcb.fat_start = myvcb.de_length + 1;

  int remaining = size - 1 - myvcb.de_length;
  int datablock_length = floor(128 * remaining / 129);
  
  myvcb.fat_length = remaining - datablock_length;
  myvcb.db_start = myvcb.fat_start + myvcb.fat_length;
  myvcb.user = getuid();
  myvcb.group = getgid();
  myvcb.mode = 0777;
  
  // Get the current time using the system's clock
  struct timespec tmp_time;
  clock_gettime(CLOCK_REALTIME, &tmp_time);

  struct timespec access_time = tmp_time;
  struct timespec modify_time = tmp_time;
  struct timespec create_time = tmp_time;

  // Write to the disk
  char tmpmem[BLOCKSIZE];
  memset(tmpmem, 0, BLOCKSIZE);
  memcpy(tmpmem, &myvcb, sizeof(vcb));

  if (dwrite(0, tmpmem) < 0) { perror("Error while writing to disk"); }

  /////////////////////////
  // Initialize a Dirent //
  /////////////////////////
  dirent de_init;
  de_init.valid = 0;

  memset(tmpmem, 0, BLOCKSIZE);
  memcpy(tmpmem, &de_init, sizeof(dirent));

  // Write out this blank directory entry as many time as we need to
  for (int j = myvcb.de_start; j < myvcb.de_length + myvcb.de_start; j++) {
    if (dwrite(j, tmpmem) < 0) { perror("Error while writing to disk"); }
  } 

  //////////////////////
  // Initialize a FAT //
  //////////////////////
  fatent fe_init;
  fe_init.used = 0;

  memset(tmpmem, 0, BLOCKSIZE);
  memcpy(tmpmem, &fe_init, sizeof(fatent));

  // Write out fat_length  blank FAT blocks starting at fat_start
  for (int k = myvcb.fat_start; k < myvcb.fat_length + myvcb.fat_start; k++) {
    if (dwrite(k, tmpmem) < 0) { perror("Error while writing to disk"); }
  }

  /* 3600: AN EXAMPLE OF READING/WRITING TO THE DISK IS BELOW - YOU'LL
           WANT TO REPLACE THE CODE BELOW WITH SOMETHING MEANINGFUL. */

  // first, create a zero-ed out array of memory  
  char *tmp = (char *) malloc(BLOCKSIZE);
  memset(tmp, 0, BLOCKSIZE);

  // now, write that to every block
/*  for (int i=0; i<size; i++) 
    if (dwrite(i, tmp) < 0) 
      perror("Error while writing to disk");
*/

  // Do not touch or move this function
  dunconnect();
}

int main(int argc, char** argv) {
  // Do not touch this function
  if (argc != 2) {
    printf("Invalid number of arguments \n");
    printf("usage: %s diskSizeInBlockSize\n", argv[0]);
    return 1;
  }

  unsigned long size = atoi(argv[1]);
  printf("Formatting the disk with size %lu \n", size);
  myformat(size);
}
