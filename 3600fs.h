/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600FS_H__
#define __3600FS_H__

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
  char* name; // BLOCKSIZE - sizeof(rest of struct)
} dirent;

typedef struct fatent_s {
  unsigned int used:1;
  unsigned int eof:1;
  unsigned int next:30;
} fatent;

#endif
