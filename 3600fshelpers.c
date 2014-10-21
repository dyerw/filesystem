#include "3600fshelpers.h"

/*
 * Find the index of a dirent with the given path.
 * Return -ENOENT if it does not exist
 * Otherwise return the index
 */

dirent* find_dirent(dirent** dirents, const char* path, int de_length) { // Valid is whether you are looking for a valid or invalid dirent
  int found = 0;
  int i;

  for (i = 0; i < de_length; i++) { // May need diff. way to get iterations length
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
