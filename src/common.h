#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>
#include <stdio.h>

typedef int Errno;

#define return_defer(value) do { result = (value); goto defer; } while (0)

#define UNIMPLEMENTED(...)                                                      \
    do {                                                                        \
        printf("%s:%d: UNIMPLEMENTED: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
        exit(1);                                                                \
    } while(0)
#define UNUSED(x) (void)(x)

#define DA_INIT_CAP 256

#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

#define da_append_many(da, new_items, new_items_count)                                      \
    do {                                                                                    \
        if ((da)->count + new_items_count > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                      \
                (da)->capacity = DA_INIT_CAP;                                               \
            }                                                                               \
            while ((da)->count + new_items_count > (da)->capacity) {                        \
                (da)->capacity *= 2;                                                        \
            }                                                                               \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));        \
            assert((da)->items != NULL && "Buy more RAM lol");                              \
        }                                                                                   \
        memcpy((da)->items + (da)->count, new_items, new_items_count*sizeof(*(da)->items)); \
        (da)->count += new_items_count;                                                     \
    } while (0)

char *temp_strdup(const char *s);
void temp_reset(void);

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

#define sb_append_buf da_append_many
#define sb_append_cstr(sb, cstr)     \
    do {                             \
        size_t n = strlen(cstr);     \
        da_append_many(sb, cstr, n); \
    } while (0)
#define sb_append_null(sb) da_append_many(sb, "", 1)

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Files;

Errno read_entire_file(const char *file_path, String_Builder *sb);
Errno write_entire_file(const char *file_path, const char *buf, size_t buf_size);
Errno read_entire_dir(const char *dir_path, Files *files);

#endif // COMMON_H_
