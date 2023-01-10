#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>

typedef int Errno;

#define return_defer(value) do { result = (value); goto defer; } while (0)

#define UNIMPLEMENTED(...)               \
    do {                                 \
        printf("%s:%d: UNIMPLEMENTED: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
        exit(1);                         \
    } while(0)
#define UNUSED(x) (void)(x)

#define DA_INIT_CAP 256

#define da_append(da, item)                                                         \
    do {                                                                            \
        if ((da)->count >= (da)->capacity) {                                        \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;  \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                      \
        }                                                                           \
                                                                                    \
        (da)->items[(da)->count++] = (item);                                        \
    } while (0)

char *temp_strdup(const char *s);
void temp_reset(void);

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Files;

char *read_entire_file(const char *file_path);
Errno read_entire_dir(const char *dir_path, Files *files);

#endif // COMMON_H_
