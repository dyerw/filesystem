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

#include "disk.h"
#include "3600fs.h"

int get_fatent_from_offset(int start_block, int offset, fatent** fatents, vcb* disk_vcb);
int get_new_fatent(fatent** fatents, vcb* disk_vcb);
void get_block(int index, char* buf);
void write_block(int index, char* buf);
void get_dirent(int index, dirent* de, vcb* disk_vcb);
void update_dirent(int index, dirent* de, vcb* disk_vcb);
int find_dirent_by_name(dirent* de, const char* path, vcb* disk_vcb);
