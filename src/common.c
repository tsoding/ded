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

char *read_entire_file(const char *file_path)
{
#define SLURP_FILE_PANIC \
    do { \
        fprintf(stderr, "Could not read file `%s`: %s\n", file_path, strerror(errno)); \
        exit(1); \
    } while (0)

    FILE *f = fopen(file_path, "r");
    if (f == NULL) SLURP_FILE_PANIC;
    if (fseek(f, 0, SEEK_END) < 0) SLURP_FILE_PANIC;

    long size = ftell(f);
    if (size < 0) SLURP_FILE_PANIC;

    char *buffer = malloc(size + 1);
    if (buffer == NULL) SLURP_FILE_PANIC;

    if (fseek(f, 0, SEEK_SET) < 0) SLURP_FILE_PANIC;

    fread(buffer, 1, size, f);
    if (ferror(f) < 0) SLURP_FILE_PANIC;

    buffer[size] = '\0';

    if (fclose(f) < 0) SLURP_FILE_PANIC;

    return buffer;
#undef SLURP_FILE_PANIC
}
