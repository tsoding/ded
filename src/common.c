#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#    define MINIRENT_IMPLEMENTATION
#    include <minirent.h>
#else
#    include <dirent.h>
#endif // _WIN32

#include "common.h"
#define ARENA_IMPLEMENTATION
#include "./arena.h"

static Arena temporary_arena = {0};

char *temp_strdup(const char *s)
{
    size_t n = strlen(s);
    char *ds = arena_alloc(&temporary_arena, n + 1);
    memcpy(ds, s, n);
    ds[n] = '\0';
    return ds;
}

void temp_reset(void)
{
    arena_reset(&temporary_arena);
}

Errno read_entire_dir(const char *dir_path, Files *files)
{
    Errno result = 0;
    DIR *dir = NULL;

    dir = opendir(dir_path);
    if (dir == NULL) {
        return_defer(errno);
    }

    errno = 0;
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
        da_append(files, temp_strdup(ent->d_name));
        ent = readdir(dir);
    }

    if (errno != 0) {
        return_defer(errno);
    }

defer:
    if (dir) closedir(dir);
    return result;
}

Errno write_entire_file(const char *file_path, const char *buf, size_t buf_size)
{
    Errno result = 0;
    FILE *f = NULL;

    f = fopen(file_path, "wb");
    if (f == NULL) return_defer(errno);

    fwrite(buf, 1, buf_size, f);
    if (ferror(f)) return_defer(errno);

defer:
    if (f) fclose(f);
    return result;
}

static Errno file_size(FILE *file, size_t *size)
{
    long saved = ftell(file);
    if (saved < 0) return errno;
    if (fseek(file, 0, SEEK_END) < 0) return errno;
    long result = ftell(file);
    if (result < 0) return errno;
    if (fseek(file, saved, SEEK_SET) < 0) return errno;
    *size = (size_t) result;
    return 0;
}

Errno read_entire_file(const char *file_path, String_Builder *sb)
{
    Errno result = 0;
    FILE *f = NULL;

    f = fopen(file_path, "r");
    if (f == NULL) return_defer(errno);

    size_t size;
    Errno err = file_size(f, &size);
    if (err != 0) return_defer(err);

    if (sb->capacity < size) {
        sb->capacity = size;
        sb->items = realloc(sb->items, sb->capacity*sizeof(*sb->items));
        assert(sb->items != NULL && "Buy more RAM lol");
    }

    fread(sb->items, size, 1, f);
    if (ferror(f)) return_defer(errno);
    sb->count = size;

defer:
    if (f) fclose(f);
    return result;
}
