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
void editor_move_char_right(Editor *e);
void editor_insert_char(Editor *e, char x);
void editor_recompute_lines(Editor *e);

#endif // EDITOR_H_
