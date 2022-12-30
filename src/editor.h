#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdlib.h>

typedef struct {
    size_t begin;
    size_t end;
} Line;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Data;

typedef struct {
    Line *items;
    size_t count;
    size_t capacity;
} Lines;

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

typedef struct {
    Data data;
    Lines lines;
    size_t cursor;
} Editor;

void editor_save_to_file(const Editor *editor, const char *file_path);
void editor_load_from_file(Editor *editor, FILE *file);

void editor_backspace(Editor *editor);
void editor_delete(Editor *editor);
size_t editor_cursor_row(const Editor *e);
void editor_move_line_up(Editor *e);
void editor_move_line_down(Editor *e);
void editor_move_char_left(Editor *e);
void editor_move_char_home(Editor *e);
void editor_move_char_end(Editor *e);
void editor_move_char_right(Editor *e);
void editor_insert_char(Editor *e, char x);
void editor_recompute_lines(Editor *e);

#endif // EDITOR_H_
