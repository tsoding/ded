#ifndef FILE_BROWSER_H_
#define FILE_BROWSER_H_

#include "common.h"

typedef struct {
    Files files;
    size_t cursor;
} File_Browser;

Errno fb_open_dir(File_Browser *fb, const char *dir_path);

#endif // FILE_BROWSER_H_
