#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "./editor.h"
#include "./common.h"
#include "./free_glyph.h"
#include "./file_browser.h"
#include "lexer.h"
#include "simple_renderer.h"
#include <ctype.h> // For isalnum
#include "evil.h"


EvilMode current_mode = NORMAL;
float zoom_factor = 3.0f;
float min_zoom_factor = 1.0;
float max_zoom_factor = 50.0;

bool isAnimated = true;
bool isWave = false;
size_t indentation = 4;

bool showLineNumbers = false;
bool highlightCurrentLineNumber = true;
bool relativeLineNumbers = false;

bool showWhitespaces = false;
bool copiedLine = false;
bool matchParenthesis = true;

bool hl_line = false;
bool superDrammtic = false;
bool showIndentationLines = true;

bool showMinibuffer = true;
bool showModeline = true;
float minibufferHeight = 21.0f;
float modelineHeight = 35.0f;
float modelineAccentWidth = 5.0f;
bool minibuffering = false;


bool BlockInsertCurosr = true;



bool extract_word_under_cursor(Editor *editor, char *word) {
    // Make a copy of cursor position to avoid modifying the actual cursor
    size_t cursor = editor->cursor;

    // Move left to find the start of the word.
    while (cursor > 0 && isalnum(editor->data.items[cursor - 1])) {
        cursor--;
    }

    // Check if the cursor is on a word or on whitespace/special character.
    if (!isalnum(editor->data.items[cursor])) return false;

    int start = cursor;

    // Move right to find the end of the word.
    while (cursor < editor->data.count && isalnum(editor->data.items[cursor])) {
        cursor++;
    }

    int end = cursor;

    // Copy the word to the provided buffer.
    // Make sure not to overflow the buffer and null-terminate the string.
    int length = end - start;
    strncpy(word, &editor->data.items[start], length);
    word[length] = '\0';

    return true;
}


void move_camera(Simple_Renderer *sr, const char* direction, float amount) {
    if(strcmp(direction, "up") == 0) {
        sr->camera_pos.y -= amount;
    } else if(strcmp(direction, "down") == 0) {
        sr->camera_pos.y += amount;
    } else if(strcmp(direction, "left") == 0) {
        sr->camera_pos.x -= amount;
    } else if(strcmp(direction, "right") == 0) {
        sr->camera_pos.x += amount;
    } else {
        printf("Invalid direction '%s'\n", direction);
    }
}


// TODO smarter
void editor_backspace(Editor *e) {
    // If in search mode, reduce the search query length
    if (e->searching) {
        if (e->search.count > 0) {
            e->search.count -= 1;
        }
    } else {
        // Check if the cursor is at the beginning or at the beginning of a line
        if (e->cursor == 0) return; // Cursor at the beginning, nothing to delete

        size_t cursor_pos = e->cursor;
        size_t row = editor_cursor_row(e);

        if (cursor_pos > e->data.count) {
            cursor_pos = e->data.count;
        }

        // Determine the characters before and after the cursor
        char char_before_cursor = (cursor_pos > 0) ? e->data.items[cursor_pos - 1] : '\0';
        char char_after_cursor = (cursor_pos < e->data.count) ? e->data.items[cursor_pos] : '\0';

        // Smart parentheses: delete both characters if they match
        if ((char_before_cursor == '(' && char_after_cursor == ')') ||
            (char_before_cursor == '[' && char_after_cursor == ']') ||
            (char_before_cursor == '{' && char_after_cursor == '}') ||
            (char_before_cursor == '\'' && char_after_cursor == '\'') ||
            (char_before_cursor == '"' && char_after_cursor == '"')) {
            memmove(&e->data.items[cursor_pos - 1], &e->data.items[cursor_pos + 1], e->data.count - cursor_pos);
            e->cursor -= 1;
            e->data.count -= 2;
        } else if (editor_is_line_empty(e, row)) {
          if (row > 0) {
            // If it's not the first line, delete the newline character from the previous line
            size_t newline_pos = e->lines.items[row - 1].end; // Position of newline character
            memmove(&e->data.items[newline_pos], &e->data.items[newline_pos + 1], e->data.count - newline_pos - 1);
            e->cursor = newline_pos; // Move cursor to the end of the previous line
            e->data.count -= 1;
          } else if (e->lines.count > 1) {
            // If it's the first line but there are more lines, delete the newline character at the end of this line
            size_t newline_pos = e->lines.items[row].end; // Position of newline character
            memmove(&e->data.items[newline_pos], &e->data.items[newline_pos + 1], e->data.count - newline_pos - 1);
            e->data.count -= 1;
            // Cursor stays at the beginning of the next line (which is now the first line)
          }
        } else if (editor_is_line_whitespaced(e, row)) {
            /* // If the line is only whitespaces */
            /* size_t line_begin = e->lines.items[row].begin; */
            /* size_t delete_length = (cursor_pos - line_begin >= indentation) ? indentation : cursor_pos - line_begin; */

            /* memmove(&e->data.items[cursor_pos - delete_length], &e->data.items[cursor_pos], e->data.count - cursor_pos); */
            /* e->cursor -= delete_length; */
            /* e->data.count -= delete_length; */

            // If the line is only whitespaces
            size_t line_begin = e->lines.items[row].begin;
            size_t line_end = e->lines.items[row].end;
            size_t whitespace_length = cursor_pos - line_begin;
            
            if (whitespace_length == indentation) {
                // If the number of whitespaces matches indentation exactly, remove the entire line
                if (row < e->lines.count - 1) {
                    memmove(&e->data.items[line_begin], &e->data.items[line_end + 1], e->data.count - line_end - 1);
                    e->data.count -= (line_end - line_begin + 1);
                    e->cursor = line_begin; // Update cursor position to the beginning of the next line
                } else if (row > 0 && e->data.items[line_begin - 1] == '\n') {
                    // If it's the last line, remove the preceding newline character
                    e->data.count -= 1;
                    memmove(&e->data.items[line_begin - 1], &e->data.items[line_end], e->data.count - line_end);
                    e->cursor = (line_begin > 1) ? line_begin - 1 : 0; // Move cursor to the end of the previous line, plus one character
                }
                // Update the cursor position if it's not the first line
                if (row > 0) {
                    e->cursor = e->lines.items[row - 1].end; // Move cursor to one character right of the end of the previous line
                    if (e->cursor > e->data.count) e->cursor = e->data.count; // Bound check
                }
            } else {
                // Original behavior for deleting whitespaces
                size_t delete_length = (whitespace_length >= indentation) ? indentation : whitespace_length;
                memmove(&e->data.items[cursor_pos - delete_length], &e->data.items[cursor_pos], e->data.count - cursor_pos);
                e->cursor -= delete_length;
                e->data.count -= delete_length;
            }
        } else {
            // Delete only the character before the cursor
            memmove(&e->data.items[cursor_pos - 1], &e->data.items[cursor_pos], e->data.count - cursor_pos);
            e->cursor -= 1;
            e->data.count -= 1;
        }
        editor_retokenize(e);
    }
}


// Unused ?
void editor_delete(Editor *e)
{
    if (e->searching) return;

    if (e->cursor >= e->data.count) return;
    memmove(
        &e->data.items[e->cursor],
        &e->data.items[e->cursor + 1],
        e->data.count - e->cursor - 1
    );
    e->data.count -= 1;
    editor_retokenize(e);
}

void editor_delete_selection(Editor *e)
{
    assert(e->selection);

    size_t begin = e->select_begin;
    size_t end = e->cursor;
    if (begin > end) {
        SWAP(size_t, begin, end);
    }

    if (end >= e->data.count) {
        end = e->data.count - 1;
    }
    if (begin == e->data.count) return;

    size_t nchars = end - begin + 1; // Correct calculation to include the end character

    memmove(
        &e->data.items[begin],
        &e->data.items[end + 1],
        e->data.count - end - 1
    );

    e->data.count -= nchars;
    e->cursor = begin; // Set cursor to the beginning of the deleted range

    editor_retokenize(e);
}


// TODO: make sure that you always have new line at the end of the file while saving
// https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_206

Errno editor_save_as(Editor *e, const char *file_path)
{
    printf("Saving as %s...\n", file_path);
    Errno err = write_entire_file(file_path, e->data.items, e->data.count);
    if (err != 0) return err;
    e->file_path.count = 0;
    sb_append_cstr(&e->file_path, file_path);
    sb_append_null(&e->file_path);
    return 0;
}

Errno editor_save(const Editor *e)
{
    assert(e->file_path.count > 0);
    printf("Saving as %s...\n", e->file_path.items);
    return write_entire_file(e->file_path.items, e->data.items, e->data.count);
}

Errno editor_load_from_file(Editor *e, const char *file_path)
{
    printf("Loading %s\n", file_path);

    e->data.count = 0;
    Errno err = read_entire_file(file_path, &e->data);
    if (err != 0) return err;

    e->cursor = 0;

    editor_retokenize(e);

    e->file_path.count = 0;
    sb_append_cstr(&e->file_path, file_path);
    sb_append_null(&e->file_path);

    // Add file path to buffer history
    if (e->buffer_history_count < MAX_BUFFER_HISTORY) {
        e->buffer_history[e->buffer_history_count++] = strdup(file_path);
    }

    return 0;
}

size_t editor_cursor_row(const Editor *e)
{
    assert(e->lines.count > 0);
    for (size_t row = 0; row < e->lines.count; ++row) {
        Line line = e->lines.items[row];
        if (line.begin <= e->cursor && e->cursor <= line.end) {
            return row;
        }
    }
    return e->lines.count - 1;
}

void editor_move_line_up(Editor *e)
{
    editor_stop_search(e);

    size_t cursor_row = editor_cursor_row(e);
    size_t cursor_col = e->cursor - e->lines.items[cursor_row].begin;
    if (cursor_row > 0) {
        Line next_line = e->lines.items[cursor_row - 1];
        size_t next_line_size = next_line.end - next_line.begin;
        if (cursor_col > next_line_size) cursor_col = next_line_size;
        e->cursor = next_line.begin + cursor_col;
    }
}

void editor_move_line_down(Editor *e)
{
    editor_stop_search(e);

    size_t cursor_row = editor_cursor_row(e);
    size_t cursor_col = e->cursor - e->lines.items[cursor_row].begin;
    if (cursor_row < e->lines.count - 1) {
        Line next_line = e->lines.items[cursor_row + 1];
        size_t next_line_size = next_line.end - next_line.begin;
        if (cursor_col > next_line_size) cursor_col = next_line_size;
        e->cursor = next_line.begin + cursor_col;
    }
}

void editor_move_char_left(Editor *e)
{
    editor_stop_search(e);
    if (e->cursor > 0) e->cursor -= 1;
}

void editor_move_char_right(Editor *e)
{
    editor_stop_search(e);
    if (e->cursor < e->data.count) e->cursor += 1;
}

void editor_move_word_left(Editor *e)
{
    editor_stop_search(e);
    while (e->cursor > 0 && !isalnum(e->data.items[e->cursor - 1])) {
        e->cursor -= 1;
    }
    while (e->cursor > 0 && isalnum(e->data.items[e->cursor - 1])) {
        e->cursor -= 1;
    }
}

void editor_move_word_right(Editor *e)
{
    editor_stop_search(e);
    while (e->cursor < e->data.count && !isalnum(e->data.items[e->cursor])) {
        e->cursor += 1;
    }
    while (e->cursor < e->data.count && isalnum(e->data.items[e->cursor])) {
        e->cursor += 1;
    }
}

void editor_insert_char(Editor *e, char x)
{
    editor_insert_buf(e, &x, 1);
}

void editor_insert_char_at(Editor *e, char c, size_t pos) {
    editor_insert_buf_at(e, &c, 1, pos);
}



void editor_insert_buf(Editor *e, char *buf, size_t buf_len)
{
    if (e->searching) {
        sb_append_buf(&e->search, buf, buf_len);
        bool matched = false;
        for (size_t pos = e->cursor; pos < e->data.count; ++pos) {
            if (editor_search_matches_at(e, pos)) {
                e->cursor = pos;
                matched = true;
                break;
            }
        }
        if (!matched) e->search.count -= buf_len;
    } else {
        if (e->cursor > e->data.count) {
            e->cursor = e->data.count;
        }

        for (size_t i = 0; i < buf_len; ++i) {
            da_append(&e->data, '\0');
        }
        memmove(
            &e->data.items[e->cursor + buf_len],
            &e->data.items[e->cursor],
            e->data.count - e->cursor - buf_len
        );
        memcpy(&e->data.items[e->cursor], buf, buf_len);
        e->cursor += buf_len;
        editor_retokenize(e);
    }
}


void editor_insert_buf_at(Editor *e, char *buf, size_t buf_len, size_t pos) {
    // Ensure the position is within bounds
    if (pos > e->data.count) {
        pos = e->data.count;
    }

    // Expand the buffer to accommodate the new text
    for (size_t i = 0; i < buf_len; ++i) {
        da_append(&e->data, '\0');
    }

    // Shift existing text to make room for the new text
    memmove(&e->data.items[pos + buf_len], &e->data.items[pos], e->data.count - pos);

    // Copy the new text into the buffer at the specified position
    memcpy(&e->data.items[pos], buf, buf_len);

    // Update the cursor position and retokenize
    e->cursor = pos + buf_len;
    editor_retokenize(e);
}

void editor_retokenize(Editor *e)
{
    // Lines
    {
        e->lines.count = 0;

        Line line;
        line.begin = 0;

        for (size_t i = 0; i < e->data.count; ++i) {
            if (e->data.items[i] == '\n') {
                line.end = i;
                da_append(&e->lines, line);
                line.begin = i + 1;
            }
        }

        line.end = e->data.count;
        da_append(&e->lines, line);
    }

    // Syntax Highlighting
    {
        e->tokens.count = 0;
        Lexer l = lexer_new(e->atlas, e->data.items, e->data.count);
        /* Lexer l = lexer_new(e->atlas, e->data.items, e->data.count, e->file_path); */
        Token t = lexer_next(&l);
        while (t.kind != TOKEN_END) {
            da_append(&e->tokens, t);
            t = lexer_next(&l);
        }
    }
}

bool editor_line_starts_with(Editor *e, size_t row, size_t col, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    if (prefix_len == 0) {
        return true;
    }
    Line line = e->lines.items[row];
    if (col + prefix_len - 1 >= line.end) {
        return false;
    }
    for (size_t i = 0; i < prefix_len; ++i) {
        if (prefix[i] != e->data.items[line.begin + col + i]) {
            return false;
        }
    }
    return true;
}

const char *editor_line_starts_with_one_of(Editor *e, size_t row, size_t col, const char **prefixes, size_t prefixes_count)
{
    for (size_t i = 0; i < prefixes_count; ++i) {
        if (editor_line_starts_with(e, row, col, prefixes[i])) {
            return prefixes[i];
        }
    }
    return NULL;
}

void editor_clipboard_copy(Editor *e)
{
    if (e->searching) return;
    if (e->selection) {
        size_t begin = e->select_begin;
        size_t end = e->cursor;
        if (begin > end) SWAP(size_t, begin, end);

        e->clipboard.count = 0;
        sb_append_buf(&e->clipboard, &e->data.items[begin], end - begin + 1);
        sb_append_null(&e->clipboard);

        if (SDL_SetClipboardText(e->clipboard.items) < 0) {
            fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        }
    }
    copiedLine = false;
}

void editor_clipboard_paste(Editor *e)
{
    char *text = SDL_GetClipboardText();
    size_t text_len = strlen(text);
    if (text_len > 0) {
        editor_insert_buf(e, text, text_len);
    } else {
        fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
    }
    SDL_free(text);
}

void editor_update_selection(Editor *e, bool shift) {
    if (e->searching) return;
    
    if (current_mode == VISUAL) {
        if (!e->selection) {
            evil_visual_char(e);
        }
    } else if (shift) {
        if (!e->selection) {
            e->selection = true;
            e->select_begin = e->cursor;
        }
    } else {
        e->selection = false;
    }
}

// search
void editor_start_search(Editor *e)
{
    if (e->searching) {
        for (size_t pos = e->cursor + 1; pos < e->data.count; ++pos) {
            if (editor_search_matches_at(e, pos)) {
                e->cursor = pos;
                break;
            }
        }
    } else {
        e->searching = true;
        if (e->selection) {
            e->selection = false;
            // TODO: put the selection into the search automatically
        } else {
            e->search.count = 0;
        }
    }
}

void editor_stop_search(Editor *e)
{
    e->searching = false;
}

void editor_stop_search_and_mark(Editor *e) {
    e->searching = false;

    e->has_mark = true;  // Mark the search result.
    e->mark_start = e->cursor;
    e->mark_end = e->cursor + e->search.count;
}

void editor_clear_mark(Editor *editor) {
    editor->has_mark = false;
    editor->mark_start = 0;  // or some other appropriate default value
    editor->mark_end = 0;    // or some other appropriate default value
}

bool editor_search_matches_at(Editor *e, size_t pos)
{
    if (e->data.count - pos < e->search.count) return false;
    for (size_t i = 0; i < e->search.count; ++i) {
        if (e->search.items[i] != e->data.items[pos + i]) {
            return false;
        }
    }
    return true;
}

void editor_move_to_begin(Editor *e)
{
    editor_stop_search(e);
    e->cursor = 0;
}

void editor_move_to_end(Editor *e)
{
    editor_stop_search(e);
    e->cursor = e->data.count;
}

void editor_move_to_line_begin(Editor *e)
{
    editor_stop_search(e);
    size_t row = editor_cursor_row(e);
    e->cursor = e->lines.items[row].begin;
}

void editor_move_to_line_end(Editor *e)
{
    editor_stop_search(e);
    size_t row = editor_cursor_row(e);
    e->cursor = e->lines.items[row].end;
}

void editor_move_paragraph_up(Editor *e)
{
    editor_stop_search(e);
    size_t row = editor_cursor_row(e);
    while (row > 0 && e->lines.items[row].end - e->lines.items[row].begin <= 1) {
        row -= 1;
    }
    while (row > 0 && e->lines.items[row].end - e->lines.items[row].begin > 1) {
        row -= 1;
    }
    e->cursor = e->lines.items[row].begin;
}

void editor_move_paragraph_down(Editor *e)
{
    editor_stop_search(e);
    size_t row = editor_cursor_row(e);
    while (row + 1 < e->lines.count && e->lines.items[row].end - e->lines.items[row].begin <= 1) {
        row += 1;
    }
    while (row + 1 < e->lines.count && e->lines.items[row].end - e->lines.items[row].begin > 1) {
        row += 1;
    }
    e->cursor = e->lines.items[row].begin;
}


bool editor_is_line_empty(Editor *e, size_t row) {
    if (row >= e->lines.count) return true; // Non-existent lines are considered empty

    return e->lines.items[row].begin == e->lines.items[row].end;
}

bool editor_is_line_whitespaced(Editor *e, size_t row) {
    if (row >= e->lines.count) return false;

    size_t line_begin = e->lines.items[row].begin;
    size_t line_end = e->lines.items[row].end;

    for (size_t i = line_begin; i < line_end; ++i) {
        if (!isspace(e->data.items[i])) {
            return false;
        }
    }
    return true;
}


ssize_t find_matching_parenthesis(Editor *editor, size_t cursor_pos) {
    // Ensure the cursor position is within the valid range
    if (cursor_pos >= editor->data.count) return -1;
    if (matchParenthesis){
        char current_char = editor->data.items[cursor_pos];
        char matching_char;
        int direction;
        
        // Check if the character at cursor is a parenthesis
        switch (current_char) {
        case '(': matching_char = ')'; direction = 1; break;
        case ')': matching_char = '('; direction = -1; break;
        case '[': matching_char = ']'; direction = 1; break;
        case ']': matching_char = '['; direction = -1; break;
        case '{': matching_char = '}'; direction = 1; break;
        case '}': matching_char = '{'; direction = -1; break;
        default: return -1; // Not on a parenthesis character
        }
        
        int balance = 1;
        size_t pos = cursor_pos;
        
        while ((direction > 0 && pos < editor->data.count - 1) || (direction < 0 && pos > 0)) {
            pos += direction;
            
            if (editor->data.items[pos] == current_char) {
                balance++;
            } else if (editor->data.items[pos] == matching_char) {
                balance--;
                if (balance == 0) {
                    return pos; // Found the matching parenthesis
                }
            }
        }
        return -1; // No matching parenthesis found
    }
}

size_t editor_row_from_pos(const Editor *e, size_t pos) {
    assert(e->lines.count > 0);
    for (size_t row = 0; row < e->lines.count; ++row) {
        Line line = e->lines.items[row];
        if (line.begin <= pos && pos <= line.end) {
            return row;
        }
    }
    return e->lines.count - 1;
}

//TODO BUG
void editor_enter(Editor *e) {
    if (e->searching) {
        editor_stop_search_and_mark(e);
        current_mode = NORMAL;
        return;
    }

    size_t row = editor_cursor_row(e);
    size_t line_end = e->lines.items[row].end;

    editor_insert_char(e, '\n');
    size_t line_begin = e->lines.items[row].begin;
    bool inside_braces = false;

    // Check if the line contains an opening brace '{'
    for (size_t i = line_begin; i < line_end; ++i) {
        char c = e->data.items[i];
        if (c == '{') {
            inside_braces = true;
            break;
        }
    }

    // Insert the same whitespace character
    for (size_t i = line_begin; i < line_end; ++i) {
        char c = e->data.items[i];
        if (c == ' ' || c == '\t') {
            editor_insert_char(e, c);
        } else {
            break;
        }
    }

    // If inside braces, perform additional steps
    if (inside_braces) {
        editor_move_line_up(e);
        editor_move_to_line_end(e);
        editor_insert_char(e, '\n');

        // Add indentation
        for (size_t i = 0; i < indentation; ++i) {
            editor_insert_char(e, ' ');
        }
    }

    e->last_stroke = SDL_GetTicks();
}


// Anchor Implementation: Initially, the anchor used a single index from the
// start of the buffer, requiring updates on text changes. To simplify, we now
// track two indices (start and end of buffer). The anchor position self-adjusts
// based on cursor's relative position, ensuring correct placement without
// modifying all text-manipulating functions still a dumb implementation.

void editor_set_anchor(Editor *editor) {
    if (editor->cursor < editor->data.count) {
        editor->has_anchor = true;
        editor->anchor_pos_from_start = editor->cursor;
        editor->anchor_pos_from_end = editor->data.count - editor->cursor;
    }
}

void editor_goto_anchor_and_clear(Editor *editor) {
    if (editor->has_anchor) {
        if (editor->cursor > editor->anchor_pos_from_start) {
            editor->cursor = editor->anchor_pos_from_start;
        } else {
            editor->cursor = editor->data.count - editor->anchor_pos_from_end;
        }
        editor->has_anchor = false;
    }
}

void editor_update_anchor(Editor *editor) {
    if (!editor->has_anchor) return;

    if (editor->cursor > editor->anchor_pos_from_start) {
        // Cursor is after the anchor, use position from the start
        editor->anchor_pos = editor->anchor_pos_from_start;
    } else {
        // Cursor is before the anchor, use position from the end
        editor->anchor_pos = editor->data.count - editor->anchor_pos_from_end;
    }
}


void editor_drag_line_down(Editor *editor) {
    size_t row = editor_cursor_row(editor);
    if (row >= editor->lines.count - 1) return; // Can't move the last line down

    Line current_line = editor->lines.items[row];
    Line next_line = editor->lines.items[row + 1];

    // Calculate lengths including the newline character
    size_t current_line_length = current_line.end - current_line.begin + 1;
    size_t next_line_length = next_line.end - next_line.begin + 1;

    // Allocate temporary buffer to hold the lines
    char *temp = malloc(current_line_length + next_line_length);
    if (!temp) {
        // Handle memory allocation error
        fprintf(stderr, "ERROR: Unable to allocate memory for line swapping.\n");
        return;
    }

    // Copy current and next lines into temp
    memcpy(temp, &editor->data.items[current_line.begin], current_line_length);
    memcpy(temp + current_line_length, &editor->data.items[next_line.begin], next_line_length);

    // Swap lines in editor's data
    memcpy(&editor->data.items[current_line.begin], temp + current_line_length, next_line_length);
    memcpy(&editor->data.items[current_line.begin + next_line_length], temp, current_line_length);

    // Free the temporary buffer
    free(temp);

    // Update cursor position
    if (editor->cursor >= current_line.begin && editor->cursor < current_line.end) {
        // The cursor is on the current line, move it down with the line
        editor->cursor += next_line_length;
    } else if (editor->cursor >= next_line.begin && editor->cursor <= next_line.end) {
        // The cursor is on the next line, move it up to the start of the current line
        editor->cursor = current_line.begin + (editor->cursor - next_line.begin);
    }

    // Update line positions in the Lines struct
    editor->lines.items[row].begin = current_line.begin;
    editor->lines.items[row].end = current_line.begin + next_line_length - 1;
    editor->lines.items[row + 1].begin = current_line.begin + next_line_length;
    editor->lines.items[row + 1].end = editor->lines.items[row + 1].begin + current_line_length - 1;

    // Retokenize
    editor_retokenize(editor);
}

void editor_drag_line_up(Editor *editor) {
    size_t row = editor_cursor_row(editor);
    if (row == 0) return; // Can't move the first line up

    Line current_line = editor->lines.items[row];
    Line previous_line = editor->lines.items[row - 1];

    // Calculate lengths including the newline character
    size_t current_line_length = current_line.end - current_line.begin + 1;
    size_t previous_line_length = previous_line.end - previous_line.begin + 1;

    // Allocate temporary buffer to hold the lines
    char *temp = malloc(current_line_length + previous_line_length);
    if (!temp) {
        // Handle memory allocation error
        fprintf(stderr, "ERROR: Unable to allocate memory for line swapping.\n");
        return;
    }

    // Copy current and previous lines into temp
    memcpy(temp, &editor->data.items[previous_line.begin], previous_line_length);
    memcpy(temp + previous_line_length, &editor->data.items[current_line.begin], current_line_length);

    // Swap lines in editor's data
    memcpy(&editor->data.items[previous_line.begin], temp + previous_line_length, current_line_length);
    memcpy(&editor->data.items[previous_line.begin + current_line_length], temp, previous_line_length);

    // Free the temporary buffer
    free(temp);

    // Update cursor position
    editor->cursor = previous_line.begin + (editor->cursor - current_line.begin);

    // Update line positions in the Lines struct
    editor->lines.items[row - 1].begin = previous_line.begin;
    editor->lines.items[row - 1].end = previous_line.begin + current_line_length - 1;
    editor->lines.items[row].begin = previous_line.begin + current_line_length;
    editor->lines.items[row].end = editor->lines.items[row].begin + previous_line_length - 1;

    // Retokenize
    editor_retokenize(editor);
}

float measure_whitespace_width(Free_Glyph_Atlas *atlas) {
    Vec2f whitespaceSize = {0.0f, 0.0f};
    free_glyph_atlas_measure_line_sized(atlas, " ", 1, &whitespaceSize);
    return whitespaceSize.x;
}

float measure_whitespace_height(Free_Glyph_Atlas *atlas) {
    Vec2f whitespaceSize = {0.0f, 0.0f};
    free_glyph_atlas_measure_line_sized(atlas, " ", 1, &whitespaceSize);
    return whitespaceSize.y;
}

void add_one_indentation_here(Editor *editor) {
    for (size_t i = 0; i < indentation; ++i) {
        editor_insert_char(editor, ' ');
    }
}

void remove_one_indentation_here(Editor *editor) {
    for (size_t i = 0; i < indentation; ++i) {
        editor_delete(editor);
    }
}

void add_one_indentation(Editor *editor) {
    size_t cursor_row = editor_cursor_row(editor);
    Line currentLineData = editor->lines.items[cursor_row];
    size_t originalCursorPosition = editor->cursor;

    // Calculate current indentation of the line
    size_t currentIndentation = 0;
    for (size_t i = currentLineData.begin; i < currentLineData.end && isspace(editor->data.items[i]); ++i) {
        currentIndentation++;
    }

    // Move cursor to the beginning of the current line
    editor->cursor = currentLineData.begin;

    // Add one level of indentation at the beginning of the line
    for (size_t i = 0; i < indentation; ++i) {
        editor_insert_char(editor, ' ');
    }

    // Adjust cursor position
    if (originalCursorPosition <= currentLineData.begin + currentIndentation) {
        // If the cursor was at or before the first non-whitespace character, move it right after the added indentation
        editor->cursor = currentLineData.begin + currentIndentation + indentation;
    } else {
        // If the cursor was on a non-whitespace character, maintain relative position
        editor->cursor = originalCursorPosition + indentation;
    }
}

void remove_one_indentation(Editor *editor) {
    size_t cursor_row = editor_cursor_row(editor);
    Line currentLineData = editor->lines.items[cursor_row];

    // Save the current cursor position
    size_t originalCursorPosition = editor->cursor;

    // Calculate current indentation of the line
    size_t currentIndentation = 0;
    size_t firstNonWhitespace = currentLineData.begin;
    while (firstNonWhitespace < currentLineData.end && isspace(editor->data.items[firstNonWhitespace])) {
        currentIndentation++;
        firstNonWhitespace++;
    }

    // Check if there's at least one indentation level to remove
    if (currentIndentation >= indentation) {
        // Move cursor to the beginning of the current line
        editor->cursor = currentLineData.begin;

        // Remove one level of indentation from the beginning of the line
        for (size_t i = 0; i < indentation; ++i) {
            editor_delete(editor); // Assuming delete removes one character.
        }

        // Adjust cursor position
        if (originalCursorPosition <= currentLineData.begin + currentIndentation) {
            // If the cursor was within the leading whitespace, move it to the first non-whitespace character
            editor->cursor = firstNonWhitespace - indentation;
        } else {
            // If the cursor was on a non-whitespace character, maintain relative position
            editor->cursor = originalCursorPosition - indentation;
        }
    }
}



// TODO slow calculation on whitespaces
void indent(Editor *editor) {
    size_t cursor_row = editor_cursor_row(editor);
    int braceLevel = 0;

    // Calculate brace level up to the current line
    for (size_t i = 0; i < cursor_row; ++i) {
        Line line = editor->lines.items[i];
        for (size_t j = line.begin; j < line.end; ++j) {
            char c = editor->data.items[j];
            if (c == '{') {
                braceLevel++;
            } else if (c == '}') {
                braceLevel = (braceLevel > 0) ? braceLevel - 1 : 0;
            }
        }
    }

    Line currentLineData = editor->lines.items[cursor_row];
    bool decreaseIndentation = false;
    size_t firstNonWhitespace = currentLineData.begin;
    bool isLineEmpty = true;
    for (size_t j = currentLineData.begin; j < currentLineData.end; ++j) {
        char c = editor->data.items[j];
        if (!isspace(c)) {
            firstNonWhitespace = j;
            isLineEmpty = false;
            if (c == '}') {
                decreaseIndentation = true;
            }
            break;
        }
    }

    // Adjust brace level for current line if it starts with a closing brace
    if (decreaseIndentation) {
        braceLevel = (braceLevel > 0) ? braceLevel - 1 : 0;
    }

    // Calculate required and current indentation
    size_t requiredIndentation = braceLevel * indentation;
    size_t currentIndentation = 0;
    for (size_t i = currentLineData.begin; i < currentLineData.end && (editor->data.items[i] == ' ' || editor->data.items[i] == '\t'); ++i) {
        currentIndentation++;
    }

    // Save the current cursor position
    size_t originalCursorPosition = editor->cursor;

    // Adjust indentation
    editor->cursor = currentLineData.begin;
    while (currentIndentation < requiredIndentation) {
        editor_insert_char(editor, ' ');
        currentIndentation++;
    }

    while (currentIndentation > requiredIndentation && currentIndentation > 0) {
        editor_delete(editor); // or evil_delete_char(editor);
        currentIndentation--;
    }

    // Adjust cursor position based on initial condition
    if (isLineEmpty || originalCursorPosition <= firstNonWhitespace) {
        // If the line is empty or the cursor was on or before the first non-whitespace character
        editor->cursor = currentLineData.begin + requiredIndentation;
    } else {
        // If the cursor was on a non-whitespace character, maintain relative position
        size_t characterOffset = originalCursorPosition - firstNonWhitespace;
        editor->cursor = currentLineData.begin + requiredIndentation + characterOffset;
    }
}


size_t find_first_non_whitespace(const char* items, size_t begin, size_t end) {
    size_t pos = begin;
    while (pos < end && isspace((unsigned char)items[pos])) {
        pos++;
    }
    return pos;
}



// CLANG FORMAT TODO
#include <stdlib.h>

int is_clang_format_installed() {
    if (system("clang-format --version") != 0) {
        return 0;
    }
    return 1;
}

void clang_format(const char *filename, const char *arguments) {
    if (!is_clang_format_installed()) {
        printf("clang-format is not installed.\n");
        return;
    }

    char command[1024];
    snprintf(command, sizeof(command), "clang-format %s %s", arguments, filename);

    // Execute the command
    int result = system(command);
    if (result != 0) {
        printf("Error executing clang-format.\n");
    }
}


// TODO select more after end brace
void select_region_from_inside_braces(Editor *editor) {
    if (editor->cursor >= editor->data.count) return;

    size_t row = editor_cursor_row(editor);
    size_t start = row;
    size_t end = row;

    // Find the start of the function
    while (start > 0) {
        start--;
        size_t line_begin = editor->lines.items[start].begin;
        size_t line_end = editor->lines.items[start].end;

        // Simple heuristic: a line ending with '{' might be the start of a function
        if (editor->data.items[line_end - 1] == '{') {
            break;
        }
    }

    // Find the end of the function
    int brace_count = 1; // Start after finding the opening brace
    while (end < editor->lines.count - 1) {
        end++;
        size_t line_begin = editor->lines.items[end].begin;
        size_t line_end = editor->lines.items[end].end;

        for (size_t i = line_begin; i < line_end; i++) {
            if (editor->data.items[i] == '{') {
                brace_count++;
            } else if (editor->data.items[i] == '}') {
                brace_count--;
                if (brace_count == 0) {
                    // Found the matching closing brace
                    goto found_end;
                }
            }
        }
    }
found_end:

    // Update the selection
    editor->selection = true;
    editor->select_begin = editor->lines.items[start].begin;
    editor->cursor = editor->lines.items[end].end;
}


// TODO should not run from anywhere just curly braces
// TODO dont move the cursor on open brace like it does for closing brace
void select_region_from_brace(Editor *editor) {
    if (editor->cursor >= editor->data.count) return;

    char current_char = editor->data.items[editor->cursor];

    if (strchr("})", current_char)) {
        // Called from a closing brace
        editor->select_begin = editor->cursor;
        evil_jump_item(editor);
        size_t row = editor_cursor_row(editor);
        editor->cursor = editor->lines.items[row].begin; // Extend to the beginning of the line
    } else if (strchr("({", current_char)) {
        // Called from an opening brace
        size_t row = editor_cursor_row(editor);
        editor->select_begin = editor->lines.items[row].begin; // Start from the beginning of the line
        evil_jump_item(editor);
        row = editor_cursor_row(editor);
        editor->cursor = editor->lines.items[row].end; // Extend to the end of the line with the closing brace
    }

    // Update the selection
    editor->selection = true;
    if (editor->select_begin > editor->cursor) {
        // Ensure select_begin is always before the cursor
        size_t temp = editor->select_begin;
        editor->select_begin = editor->cursor;
        editor->cursor = temp;
    }
}


// TODO select_function






