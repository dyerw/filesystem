/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need 
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309

#include <time.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/statfs.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"
#include "disk.h"
#include "3600fshelpers.h"

vcb* disk_vcb;
dirent** dirents;
fatent** fatents;

/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that. 
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
static void* vfs_mount(struct fuse_conn_info *conn) {
  fprintf(stderr, "vfs_mount called\n");

  // Do not touch or move this code; connects the disk
  dconnect();

  /* 3600: YOU SHOULD ADD CODE HERE TO CHECK THE CONSISTENCY OF YOUR DISK
           AND LOAD ANY DATA STRUCTURES INTO MEMORY */

  // Get VCB Data
  char vcb_data[BLOCKSIZE];
  if (dread(0, vcb_data) < 0) { fprintf(stderr, "dread failed for block 0\n"); }
  // Cast our data from disk into a vcb structure
  disk_vcb = calloc(1, sizeof(vcb));
  memcpy(disk_vcb, &vcb_data, sizeof(vcb));

  printf("MAGIC: %i\n", disk_vcb->magic); 
  printf("BLOCKSIZE: %i\n", disk_vcb->blocksize); 
  printf("DE_START: %i\n", disk_vcb->de_start); 
  printf("DE_LENGTH: %i\n", disk_vcb->de_length); 
  printf("FAT_START: %i\n", disk_vcb->fat_start); 
  printf("FAT_LENGTH: %i\n", disk_vcb->fat_length); 
  printf("DB_START: %i\n", disk_vcb->db_start); 

  if (disk_vcb->magic != 666) { fprintf(stderr, "magic number mismatch\n"); exit(1); }
 
  // Get Directory Entries
  // Start reading blocks at the dirent start, until the start + the length
  for (int i = disk_vcb->de_start; i < disk_vcb->de_start + disk_vcb->de_length; i++) {
    // Read the block off the disk into a character buffer
    char dirent_buf[BLOCKSIZE];
    if (dread(i, dirent_buf) < 0) { fprintf(stderr, "dread failed\n"); }

    // Move the character buffer into a dirent struct
    dirent* tmp = calloc(1, sizeof(dirent));
    memcpy(tmp, &dirent_buf, sizeof(dirent));

    // Place that struct into the global array
    // i - disk_vcb->de_start + 1 is the amount of entries in the array
    dirents = realloc(dirents, (i - disk_vcb->de_start + 1) * sizeof(dirent*));
    dirents[i - disk_vcb->de_start] = tmp;
  }  

  // Get FAT Entries
  for (int j = disk_vcb->fat_start; j < disk_vcb->fat_start + disk_vcb->fat_length; j++) {
    char fat_buf[BLOCKSIZE];
    if (dread(j, fat_buf) < 0) { fprintf(stderr, "dread failed\n"); }
    fatent* fat_tmp = calloc(1, sizeof(fatent));
    memcpy(fat_tmp, &fat_buf, sizeof(fatent));

    fatents = realloc(fatents, (j - disk_vcb->fat_start + 1) * sizeof(fatent*));
    fatents[j - disk_vcb->fat_start] = fat_tmp;
  }
  return NULL;
}

/*
 * Called when your file system is unmounted.
 *
 */
static void vfs_unmount (void *private_data) {
  fprintf(stderr, "vfs_unmount called\n");

  /* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
           ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
           KEEP DATA CACHED THAT'S NOT ON DISK */

  char tmp_block[BLOCKSIZE];
  // Write valid dirents back to disk
  for (int i = 0; i < disk_vcb->de_length; i++) {
    if (dirents[i]->valid) {
      memset(tmp_block, 0, BLOCKSIZE);
      memcpy(tmp_block, dirents[i], sizeof(dirent));
      dwrite(disk_vcb->de_start + i, tmp_block);
    }
  }

  // Do not touch or move this code; unconnects the disk
  dunconnect();
}

/* 
 *
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
static int vfs_getattr(const char *path, struct stat *stbuf) {
  /*fprintf(stderr, "vfs_getattr called\n"); */
  
  if (strrchr(path, '/') > path) {
    fprintf(stderr, "Unable to get_attr on a multilevel dir\n");
  }

  // Do not mess with this code 
  stbuf->st_nlink = 1; // hard links
  stbuf->st_rdev  = 0;
  stbuf->st_blksize = BLOCKSIZE;

  /* 3600: YOU MUST UNCOMMENT BELOW AND IMPLEMENT THIS CORRECTLY */
  dirent* tmp_de = find_dirent(dirents, path, disk_vcb->de_length);

  // If file DNE
  if (tmp_de == NULL) {
    return -ENOENT;
  }
 
  else {
    // if (The path represents the root directory)
    if (*path == '/' && *(path + 1) == '\0') { //if the first char is a '/', it is referencing the root dir
      stbuf->st_mode     = 0777 | S_IFDIR;
      stbuf->st_gid      = disk_vcb->group;
      stbuf->st_uid      = disk_vcb->user;
      stbuf->st_atime    = disk_vcb->access_time.tv_sec; // access time 
      stbuf->st_mtime    = disk_vcb->modify_time.tv_sec; // modify time
      stbuf->st_ctime    = disk_vcb->create_time.tv_sec; // create time
      stbuf->st_size     = sizeof(vcb); // file size
      stbuf->st_blocks   = 1; // a vcb is one block
    } else {
      stbuf->st_mode    = tmp_de->mode | S_IFREG; 
      stbuf->st_uid     = tmp_de->user; // file uid
      stbuf->st_gid     = tmp_de->group; // file gid
      stbuf->st_atime   = tmp_de->access_time.tv_sec; // access time 
      stbuf->st_mtime   = tmp_de->modify_time.tv_sec; // modify time
      stbuf->st_ctime   = tmp_de->create_time.tv_sec; // create time
      stbuf->st_size    = tmp_de->size; // file size
      stbuf->st_blocks  = ceil(tmp_de->size / BLOCKSIZE); // file size in blocks
    }
    return 0;
  }
}

/** Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return 
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *			   
 * Your solution should not need to touch fi
 *
 */
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{

    // fprintf(stderr, "vfs_readdir called\n");  

    // If the given path is not the root of the file system, throw error
    if (strcmp(path, "/") != 0) {
      return -1;
    }

    for (int i = 0; i < disk_vcb->de_length; i++) {
      if (dirents[i]->valid) {
        if (*dirents[i]->name == '/') {
          filler(buf, (dirents[i]->name + 1), NULL, 0);
        } else {
          filler(buf, dirents[i]->name, NULL, 0);
        }
      }
    }

    return 0;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create 
 * will create a new file named myFile in the /a/b directory.
 *
 */
static int vfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    fprintf(stderr, "vfs_create called\n");  
    if (strrchr(path, '/') > path) {
      fprintf(stderr, "Unable to create on a multilevel dir\n");
      return -1;
    }

    // Find an unused dirent
    int full = 1;
    int i;
    for (i=0; i < disk_vcb->de_length; i++){
      if (dirents[i]->valid == 0) {
        full = 0;
        break;
      }
    }

    // Throw error if all dirents full
    if (full == 1) {
      fprintf(stderr, "There are no more available dirents\n");
      return -1;
    }
    
    // Fill an invalid dirent for this new file
    dirents[i]->valid = 1;
    dirents[i]->mode = mode;
    dirents[i]->user = geteuid();
    dirents[i]->group = getegid();
    dirents[i]->size = 0;
    struct timespec mytime;
    clock_gettime(CLOCK_REALTIME, &mytime);
    dirents[i]->create_time = mytime;
    dirents[i]->modify_time = mytime;
    dirents[i]->access_time = mytime;
    memcpy(dirents[i]->name, path, strlen(path) + 1);
    dirents[i]->first_block = get_new_fatent(fatents, disk_vcb);

    return 0;
}

/*
 * The function vfs_read provides the ability to read data from 
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return 
 * value is the amount of bytes actually read; if the file is 
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read. 
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
  fprintf(stderr, "vfs_read called\n");  
  
  // Keep track of the original size  
  int og_size = size;

  // Get this file's directory entry
  dirent* f_dirent = find_dirent(dirents, path, disk_vcb->de_length);
  if (f_dirent == NULL) return -ENOENT; // Error check for can't find file

  // Get the correct Fat Entry for the given offset
  int current_index = get_fatent_from_offset(f_dirent->first_block, offset, fatents, disk_vcb);
  fatent* current_block = fatents[current_index];

  // Read from the disk until we've satisfied the given size
  while (size > 0) {
    char current_buffer[BLOCKSIZE];

    // Get the current block into memory
    dread(disk_vcb->db_start + current_index, current_buffer);

    // Copy from the current block until the end or the size,
    // whichever is smaller, starting at the offset % BLOCKSIZE
    int block_offset = offset % BLOCKSIZE;
    char* start_address = current_buffer + block_offset;
    int read_size = BLOCKSIZE - block_offset;
    if (read_size > size) read_size = size;

    // Start writing into our buffer argument where we left off
    // this will be the number of bytes we've read so far or 
    // original bytes - current bytes
    char* buff_start = buf + (og_size - size);
    memcpy(buff_start, start_address, read_size);

    size -= read_size;

    if (current_block->eof) {
       break;
    } else {
      current_index = current_block->next;
      current_block = fatents[current_index];
    }

  } 

  return og_size - size;
}

/*
 * The function vfs_write will attempt to write 'size' bytes from 
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */
static int vfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{

  /* 3600: NOTE THAT IF THE OFFSET+SIZE GOES OFF THE END OF THE FILE, YOU
           MAY HAVE TO EXTEND THE FILE (ALLOCATE MORE BLOCKS TO IT). */
  fprintf(stderr, "vfs_write called\n");

  // The original values of these variables are necessary
  // after we modify them during the write
  int og_offset = offset;
  int og_size = size;
  
  // Get this file's directory entry
  dirent* f_dirent = find_dirent(dirents, path, disk_vcb->de_length);
  if (f_dirent == NULL) return -ENOENT; // Error check for can't find file

  // Get the start block 
  int current_index = get_fatent_from_offset(f_dirent->first_block, offset, fatents, disk_vcb); 
  fatent* current_block = fatents[current_index]; 

  // Keep writing to blocks until we've written as much as we've been asked
  while (size > 0) {
    // Get the current contents of the block we are writing to into a buffer
    char current_buffer[BLOCKSIZE];
    dread(disk_vcb->db_start + current_index, current_buffer);

    // Copy from our buf argument starting at offset % BLOCKSIZE
    // until BLOCKSIZE - (offset % BLOCKSIZE) or size, whichever is larger
    int block_offset = offset % BLOCKSIZE;
    char* start_address = current_buffer + block_offset;
    int copy_size = BLOCKSIZE - block_offset; // When the data fills the rest of the block
    if (copy_size > size) copy_size = size; // When the data does not fill it

    memcpy(start_address, buf, copy_size);

    // We now write the modified block back to disk
    dwrite(disk_vcb->db_start + current_index, current_buffer);
 
    // We want to start writing in the next block from where we left off
    buf = buf + copy_size;

    // We want size to reflect that we just wrote some of the buf
    size = size - copy_size;

    // We want to update the offset to write as much further as we just wrote
    offset = offset + copy_size;

    // If this isn't the last block to be written, we must jump to the next FAT Entry
    // and if it does not exist, get a new one
    if (size > 0) {
      if (!current_block->eof) {
        current_index = current_block->next;
        current_block = fatents[current_index];
      } else {
        current_index = get_new_fatent(fatents, disk_vcb);
        if (current_index == -1) return -ENOSPC;
        current_block->next = current_index;
        current_block->eof = 0;
        current_block = fatents[current_index];
      }
    }
  }

  // Update size metadata
  int current_size = f_dirent->size;
  int new_size;
  if (og_offset + og_size > current_size) {
    new_size = og_offset + og_size;
  } else {
    new_size = current_size;
  }
  f_dirent->size = new_size;

  return og_size;
}

/**
 * This function deletes the last component of the path (e.g., /a/b/c you 
 * need to remove the file 'c' from the directory /a/b).
 */
static int vfs_delete(const char *path)
{
    fprintf(stderr, "vfs_delete called\n");  
    

    // If the path given is a multilevel path 
    if (strrchr(path, '/') > path) {
      fprintf(stderr, "Unable to create on a multilevel dir\n");
      return -1;
    }

    /* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
             AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */
    dirent* tmp_de = find_dirent(dirents, path, disk_vcb->de_length);

    // If file DNE
    if (tmp_de == NULL) {
      return -ENOENT;
    }

    tmp_de->valid = 0;
    // mark the fat entry as unused
    unsigned int tmp = tmp_de->first_block;
    while (!fatents[tmp]->eof) {
      fatents[tmp]->used = 0;
      tmp = fatents[tmp]->next;
    }

    if (fatents[tmp]->eof) fatents[tmp]->used = 0;

    return 0;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to)
{
    //fprintf(stderr, "vfs_rename called\n");  

    // If the destination path already exists, delete that file
    dirent* tmp_de_to = find_dirent(dirents, to, disk_vcb->de_length);
    if (tmp_de_to != NULL) {
      vfs_delete(to);
    }

    // If source file DNE
    dirent* tmp_de_from = find_dirent(dirents, from, disk_vcb->de_length);
    if (tmp_de_from == NULL) {
      return -ENOENT;
    }

    strcpy(tmp_de_from->name, to);

    return 0;
}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.  
 * Only the permission bits of mode should be examined 
 * (basically, the last 16 bits).  You should do something like
 * 
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode)
{
    //fprintf(stderr, "vfs_chmod called\n");  
   
    dirent* tmp_de = find_dirent(dirents, file, disk_vcb->de_length);
    // If file DNE
    if (tmp_de == NULL) {
      return -ENOENT;
    }

    tmp_de->mode = mode;

    return 0;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid)
{
    //fprintf(stderr, "vfs_chown called\n");  
    dirent* tmp_de = find_dirent(dirents, file, disk_vcb->de_length);
    // If file DNE
    if (tmp_de == NULL) {
      return -ENOENT;
    }

    tmp_de->user = uid;
    tmp_de->group = gid;

    return 0;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2])
{
    //fprintf(stderr, "vfs_utimens called\n");
    
    dirent* tmp_de = find_dirent(dirents, file, disk_vcb->de_length);
    // If file DNE
    if (tmp_de == NULL) {
      return -ENOENT;
    }

    tmp_de->access_time = ts[0];
    tmp_de->modify_time = ts[1];

    return 0;
}

/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset)
{

  /* 3600: NOTE THAT ANY BLOCKS FREED BY THIS OPERATION SHOULD
           BE AVAILABLE FOR OTHER FILES TO USE. */
  //  fprintf(stderr, "vfs_truncate called\n");  
    dirent* tmp_de = find_dirent(dirents, file, disk_vcb->de_length);
    // If file DNE
    if (tmp_de == NULL) {
      return -ENOENT;
    }

    // If offset > file_size, throw an error
    if (offset > tmp_de->size) {
      fprintf(stderr, "Offset is greater than file size. Cannot truncate\n");
      return -1;
    }

    // Mark all fatents (for this file) past the offset fatent to unused
    int fe_index = get_fatent_from_offset(tmp_de->first_block, offset, fatents, disk_vcb);
    int i = fe_index;
    while (!fatents[i]->eof) {
      i = fatents[i]->next;
      fatent* fe = fatents[i];
      fe->used = 0;
    }
    
    // Mark the offset block as eof
    fatents[fe_index]->eof = 1;

    // write the block back to disk with the anything past offset zeroed out
    char buf[BLOCKSIZE];
    dread(fe_index, buf);
    memset(buf + (offset % BLOCKSIZE), 0, BLOCKSIZE - (offset % BLOCKSIZE));
    dwrite(fe_index, buf);
    
    // Set size equal to the offset
    tmp_de->size = offset;
    return 0;
}


/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add 
 *
 *     .mkdir	 = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
    .init    = vfs_mount,
    .destroy = vfs_unmount,
    .getattr = vfs_getattr,
    .readdir = vfs_readdir,
    .create	 = vfs_create,
    .read	 = vfs_read,
    .write	 = vfs_write,
    .unlink	 = vfs_delete,
    .rename	 = vfs_rename,
    .chmod	 = vfs_chmod,
    .chown	 = vfs_chown,
    .utimens	 = vfs_utimens,
    .truncate	 = vfs_truncate,
};

int main(int argc, char *argv[]) {
    /* Do not modify this function */
    umask(0);
    if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
      printf("Usage: ./3600fs -s -d <dir>\n");
      exit(-1);
    }
    return fuse_main(argc, argv, &vfs_oper, NULL);
}
