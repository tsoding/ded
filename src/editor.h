#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdlib.h>
#include "common.h"
#include "free_glyph.h"
#include "simple_renderer.h"

#include <SDL2/SDL.h>

typedef struct {
    size_t begin;
    size_t end;
} Line;

typedef struct {
    Line *items;
    size_t count;
    size_t capacity;
} Lines;

typedef struct {
    String_Builder data;
    Lines lines;
    String_Builder file_path;

    bool selection;
    size_t select_begin;
    size_t cursor;

    Uint32 last_stroke;
} Editor;

Errno editor_save_as(Editor *editor, const char *file_path);
Errno editor_save(const Editor *editor);
Errno editor_load_from_file(Editor *editor, const char *file_path);

void editor_backspace(Editor *editor);
void editor_delete(Editor *editor);
size_t editor_cursor_row(const Editor *e);
void editor_move_line_up(Editor *e);
void editor_move_line_down(Editor *e);
void editor_move_char_left(Editor *e);
void editor_move_char_right(Editor *e);
void editor_insert_char(Editor *e, char x);
void editor_recompute_lines(Editor *e);
void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor);
void editor_update_selection(Editor *e, bool shift);

#endif // EDITOR_H_
