/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file is the file for reading/writing the virual disk.
 * You **SHOULD NOT** touch anything in this file.
 */

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "disk.h"

const int BLOCKSIZE = 512;

int fd = -1;
int dreads = 0;
int dwrites = 0;

/***************** DO NOT MODIFY THE FUNCTIONS BELOW ***************/

/* Creates and onnects the disk */
int dcreate_connect() {
  fd = open(DISKFILE, O_RDWR | O_CREAT, 0777);
  if (fd < 0) {
    perror("dconnect unable to access the disk file");
    return 1;
  }

  dreads = 0;
  dwrites = 0;
  return 0;
}

/* Connects the disk */
int dconnect() {
  fd = open(DISKFILE, O_RDWR);
  if (fd < 0) {
    perror("dconnect unable to access the disk file");
    return 1;
  }

  dreads = 0;
  dwrites = 0;
  return 0;
}

/* Unconnects the disk */
int dunconnect() {
  if (fd > 0) {
    close(fd);
    fd = -1;

    printf("Disk usage: %d reads, %d writes\n", dreads, dwrites);
    return 0;
  } else {
    perror("disk is not connected");
  }

  return 1;
}

/* Read a block from disk */
int dread(int blocknum, char *buf) {
//  printf("DEBUG: Reading from block %d\n", blocknum);

  if (fd == -1) {
    perror("disk is not connected");
    return -1;
  }
 
  if(lseek(fd, blocknum*BLOCKSIZE, SEEK_SET)<0)
    return -2;

  if (read(fd, buf, BLOCKSIZE) != BLOCKSIZE) {
    perror("dread");
    return -3;
  }

  dreads++;

  return BLOCKSIZE;
}

/* Write a block to disk */
int dwrite(int blocknum, char *buf) {
//  printf("DEBUG: Writing to block %d\n", blocknum);

  if (fd == -1) {
    perror("disk is not connected");
    return -1;
  }
 
  if(lseek(fd, blocknum*BLOCKSIZE, SEEK_SET)<0)
    return -2;

  if (write(fd, buf, BLOCKSIZE) != BLOCKSIZE) {
    perror("dwrite");
    return -3;
  }

  dwrites++;

  return BLOCKSIZE;
}

