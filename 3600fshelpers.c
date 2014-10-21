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
