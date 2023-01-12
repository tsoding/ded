#include <string.h>
#include "file_browser.h"

static int file_cmp(const void *ap, const void *bp)
{
    const char *a = *(const char**)ap;
    const char *b = *(const char**)bp;
    return strcmp(a, b);
}

Errno fb_open_dir(File_Browser *fb, const char *dir_path)
{
    fb->files.count = 0;
    fb->cursor = 0;
    Errno err = read_entire_dir(dir_path, &fb->files);
    if (err != 0) {
        return err;
    }
    qsort(fb->files.items, fb->files.count, sizeof(*fb->files.items), file_cmp);
    return 0;
}
