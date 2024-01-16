#ifndef EDITOR_H_
#define EDITOR_H_

#include <stddef.h>
#include <stdlib.h>
#include "common.h"
#include "free_glyph.h"
#include "simple_renderer.h"
#include "lexer.h"
#include <stdbool.h>
#include <SDL2/SDL.h>


extern bool isAnimated;
extern size_t indentation;
extern float zoom_factor;
extern float min_zoom_factor;
extern float max_zoom_factor;
extern bool showLineNumbers;
extern bool isWave;
extern bool showWhitespaces;
extern bool copiedLine;
extern bool hl_line;
extern bool relativeLineNumbers;
extern bool highlightCurrentLineNumber;
extern bool matchParenthesis;
extern bool superDrammtic;
extern bool showIndentationLines;

extern bool showMinibuffer;
extern bool showModeline;
extern float minibufferHeight;
extern float modelineHeight;
extern float modelineAccentWidth;
extern bool minibuffering;

extern bool BlockInsertCurosr;

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
    EMACS,
    NORMAL,
    INSERT,
    VISUAL,
    VISUAL_LINE,
} EvilMode;

extern EvilMode current_mode;


#define MAX_BUFFER_HISTORY 100

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

    bool has_mark;            // Indicates if there's a marked search result.
    size_t mark_start;        // Start of marked search result. TODO support multiple marks
    size_t mark_end;          // End of marked search result.

    Uint32 last_stroke;

    String_Builder clipboard;

    bool has_anchor;
    size_t anchor_pos_from_start;
    size_t anchor_pos_from_end;
    size_t anchor_pos;
    

    char *buffer_history[MAX_BUFFER_HISTORY];
    int buffer_history_count;
    int buffer_index;

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
void editor_update_selection(Editor *e, bool shift);
void editor_clipboard_copy(Editor *e);
void editor_clipboard_paste(Editor *e);



void editor_start_search(Editor *e);
void editor_stop_search(Editor *e);
bool editor_search_matches_at(Editor *e, size_t pos);


// ADDED
void editor_stop_search_and_mark(Editor *e);
void editor_clear_mark(Editor *editor);
void move_camera(Simple_Renderer *sr, const char* direction, float amount);

void editor_insert_buf_at(Editor *e, char *buf, size_t buf_len, size_t pos);
void editor_insert_char_at(Editor *e, char c, size_t pos);

ssize_t find_matching_parenthesis(Editor *editor, size_t cursor_pos);
void editor_enter(Editor *e);

void editor_set_anchor(Editor *editor);
void editor_goto_anchor_and_clear(Editor *editor);
void editor_update_anchor(Editor *editor);

void editor_drag_line_down(Editor *editor);
void editor_drag_line_up(Editor *editor);

void add_one_indentation_here(Editor *editor);
void add_one_indentation(Editor *editor);
void remove_one_indentation(Editor *editor);
void indent(Editor *editor);
void select_region_from_brace(Editor *editor);
void select_region_from_inside_braces(Editor *editor);


// UTILITY
size_t editor_row_from_pos(const Editor *e, size_t pos);
bool extract_word_under_cursor(Editor *editor, char *word);
bool editor_is_line_empty(Editor *e, size_t row);
bool editor_is_line_whitespaced(Editor *e, size_t row);
float measure_whitespace_width(Free_Glyph_Atlas *atlas);
float measure_whitespace_height(Free_Glyph_Atlas *atlas);
size_t find_first_non_whitespace(const char* items, size_t begin, size_t end);



#endif // EDITOR_H_
