/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file is the header file for the virual disk.
 * You **SHOULD NOT** touch anything in this file.
 */

#ifndef __DISK_H__
#define __DISK_H__

extern const int BLOCKSIZE;

/* This is the name of the file you will use for your disk.
   Currently, it looks in the current directory.  You may want to
   replace this with an absolute path if running in daemon
   mode. Should be ok as a relative path if running in debug mode */
#define DISKFILE "MYDISK"

/* Creates and connects the disk.
 * If a disk already exists, a new one *will not* be created, and
 * the old one will be overwritten.
 * Returns 0 on success, something else on failure.
 */
int dcreate_connect();

/* Connects the disk 
 * Returns 0 on success, something else on failure.
 */
int dconnect();

/* Unconnects the disk 
 * Returns 0 on success, something else on failure.
 */
int dunconnect();

/* Read a block from disk 
 * Returns the number of bytes read, or a value less than 0 on error.
 */
int dread(int blocknum, char *buf);

/* Write a block to disk 
 * Returns the number of bytes written, or a value less than 0 on error.
 */
int dwrite(int blocknum, char *buf);

#endif

