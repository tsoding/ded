#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdlib.h>
#include "common.h"
#include "free_glyph.h"
#include "simple_renderer.h"
#include "lexer.h"

#include <stdbool.h>

#include <SDL2/SDL.h>


extern bool is_animated;
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
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef enum {
    NORMAL,
    INSERT,
    VISUAL,
    VISUAL_LINE,
    COMMAND,
    CHORDING
} EvilMode;

extern EvilMode current_mode;

typedef struct {
    Vec4f cursor;
    Vec4f text;
    Vec4f background;
    Vec4f logic;
    Vec4f comment;
    Vec4f hashtag;
    Vec4f string;
    Vec4f selection;
    Vec4f search;
    Vec4f todo;
    Vec4f fixme;
    Vec4f note;
    Vec4f marks;
    Vec4f fb_selection;
} Theme;


typedef struct {
    Free_Glyph_Atlas *atlas;

    String_Builder data;
    Lines lines;
    Tokens tokens;
    String_Builder file_path;

    bool searching;
    String_Builder search;

    bool selection;
    size_t select_begin;
    size_t cursor;

    /* EvilMode mode; // TODO */
    bool has_mark;            // Indicates if there's a marked search result.
    size_t mark_start;        // Start of marked search result.
    size_t mark_end;          // End of marked search result.


    Uint32 last_stroke;

    String_Builder clipboard;
} Editor;

Errno editor_save_as(Editor *editor, const char *file_path);
Errno editor_save(const Editor *editor);
Errno editor_load_from_file(Editor *editor, const char *file_path);

void editor_backspace(Editor *editor);
void editor_delete(Editor *editor);
void editor_delete_selection(Editor *editor);
size_t editor_cursor_row(const Editor *e);

void editor_move_line_up(Editor *e);
void editor_move_line_down(Editor *e);
void editor_move_char_left(Editor *e);
void editor_move_char_right(Editor *e);
void editor_move_word_left(Editor *e);
void editor_move_word_right(Editor *e);

void editor_move_to_begin(Editor *e);
void editor_move_to_end(Editor *e);
void editor_move_to_line_begin(Editor *e);
void editor_move_to_line_end(Editor *e);

void editor_move_paragraph_up(Editor *e);
void editor_move_paragraph_down(Editor *e);

void editor_insert_char(Editor *e, char x);
void editor_insert_buf(Editor *e, char *buf, size_t buf_len);
void editor_retokenize(Editor *e);
void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor);
void editor_update_selection(Editor *e, bool shift);
void editor_clipboard_copy(Editor *e);
void editor_clipboard_paste(Editor *e);
void editor_start_search(Editor *e);
void editor_stop_search(Editor *e);
bool editor_search_matches_at(Editor *e, size_t pos);

// ADDED
void editor_stop_search_and_mark(Editor *e);
void editor_search_next(Editor *e);
void editor_search_previous(Editor *e);
extern float zoom_factor;
extern bool showLineNumbers;

// THEME
extern Theme themes[];
extern int currentThemeIndex;
void initialize_themes();
#define CURRENT_THEME (themes[currentThemeIndex])

void theme_next(int *currentThemeIndex);
void theme_previous(int *currentThemeIndex);


#endif // EDITOR_H_
