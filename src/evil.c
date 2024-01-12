#include "evil.h"
#include "editor.h"

void evil_open_below(Editor *editor) {
    size_t row = editor_cursor_row(editor);
    size_t line_begin = editor->lines.items[row].begin;
    size_t line_end = editor->lines.items[row].end;

    editor_move_to_line_end(editor);
    editor_insert_char(editor, '\n');

    // Copy indentation
    for (size_t i = line_begin; i < line_end; ++i) {
        char c = editor->data.items[i];
        if (c == ' ' || c == '\t') {
            editor_insert_char(editor, c);
        } else {
            break;
        }
    }
}

void evil_open_above(Editor *editor) {
    size_t row = editor_cursor_row(editor);

    // Determine the current line's start and end for capturing indentation
    size_t line_begin = editor->lines.items[row].begin;
    size_t line_end = editor->lines.items[row].end;

    // Capture the indentation of the current line in a local array
    char indentation[128]; // Assuming 128 characters is enough for indentation
    size_t indentIndex = 0;
    for (size_t i = line_begin; i < line_end && indentIndex < sizeof(indentation) - 1; ++i) {
        char c = editor->data.items[i];
        if (c == ' ' || c == '\t') {
            indentation[indentIndex++] = c;
        } else {
            break;
        }
    }
    indentation[indentIndex] = '\0'; // Null-terminate the string

    // Insert a newline at the beginning of the current line
    editor_move_to_line_begin(editor);
    editor_insert_char(editor, '\n');
    editor_move_line_up(editor);

    // Apply the captured indentation
    for (size_t i = 0; i < indentIndex; ++i) {
        editor_insert_char(editor, indentation[i]);
    }
}

void evil_jump_item(Editor *editor) {
    if (editor->cursor >= editor->data.count) return;

    char current_char = editor->data.items[editor->cursor];
    ssize_t matching_pos = -1;

    // Check if the current cursor position is a parenthesis
    if (strchr("()[]{}", current_char)) {
        matching_pos = find_matching_parenthesis(editor, editor->cursor);
    } else {
        // If not, search for a parenthesis on the current line
        size_t row = editor_cursor_row(editor);
        size_t line_begin = editor->lines.items[row].begin;
        size_t line_end = editor->lines.items[row].end;

        for (size_t pos = line_begin; pos < line_end; ++pos) {
            current_char = editor->data.items[pos];
            if (strchr("()[]{}", current_char)) {
                matching_pos = find_matching_parenthesis(editor, pos);
                if (matching_pos != -1) {
                    break;
                }
            }
        }
    }

    // Move the cursor to the matching parenthesis
    if (matching_pos != -1) {
        editor->cursor = matching_pos;
    }
}

// TODO when there is a {} dont add the space
void evil_join(Editor *e) {
    size_t row = editor_cursor_row(e);
    if (row >= e->lines.count - 1) return; // Exit if on the last line

    // Get the current line and the next line
    size_t current_line_end = e->lines.items[row].end;
    size_t next_line_begin = e->lines.items[row + 1].begin;
    size_t next_line_end = e->lines.items[row + 1].end;


    // Check if the current line is empty or only has whitespaces
    bool only_whitespaces = true;
    for (size_t i = e->lines.items[row].begin; i < current_line_end; ++i) {
        if (!isspace(e->data.items[i])) {
            only_whitespaces = false;
            break;
        }
    }

    if (only_whitespaces) {
        // Current line is empty or has only whitespaces, delete the line
        size_t length_to_move = e->data.count - current_line_end;
        memmove(&e->data.items[e->lines.items[row].begin],
                &e->data.items[next_line_begin],
                length_to_move);
        e->data.count -= (next_line_begin - e->lines.items[row].begin);
        editor_retokenize(e);
        return;
    }

    // Check if the current line ends in a newline character
    if (e->data.items[current_line_end] == '\n') {
        // Skip leading spaces on the next line
        while (next_line_begin < next_line_end &&
               isspace(e->data.items[next_line_begin])) {
            next_line_begin++;
        }

        // Calculate the length to move in memmove
        size_t length_to_move = e->data.count - next_line_begin;

        // Move the data from the next line start to the current line end
        memmove(&e->data.items[current_line_end + 1],
                &e->data.items[next_line_begin],
                length_to_move);

        // Adjust the total count of characters in the buffer
        e->data.count -= (next_line_begin - current_line_end - 1);

        // Insert a single space to separate the lines
        e->data.items[current_line_end] = ' ';
    }

    editor_retokenize(e);
}

void evil_yank_line(Editor* editor) {
    size_t start = editor->cursor;
    while (start > 0 && editor->data.items[start - 1] != '\n') {
        start--;
    }

    size_t end = start;
    while (end < editor->data.count && editor->data.items[end] != '\n') {
        end++;
    }

    if (start < end) {
        editor->clipboard.count = 0;
        sb_append_buf(&editor->clipboard, &editor->data.items[start], end - start);
        sb_append_null(&editor->clipboard);

        if (SDL_SetClipboardText(editor->clipboard.items) < 0) {
            fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        }
    }
    copiedLine = true;
}

void evil_paste_after(Editor* editor) {
    if (!copiedLine) {
        return; // Do nothing if no line has been copied
    }

    char *text = SDL_GetClipboardText();
    if (!text) {
        fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        return;
    }

    size_t text_len = strlen(text);
    if (text_len > 0) {
        // Find the end of the current line
        size_t end = editor->cursor;
        while (end < editor->data.count && editor->data.items[end] != '\n') {
            end++;
        }

        // If not at the end of the file, move to the start of the next line
        if (end < editor->data.count) {
            end++;
        }

        // Insert the text from the clipboard
        editor_insert_buf_at(editor, text, text_len, end);

        // If the pasted text does not end with a newline, add one
        if (text[text_len - 1] != '\n') {
            editor_insert_buf_at(editor, "\n", 1, end + text_len);
        }

        // Move cursor to the first non-space character of the pasted line
        editor->cursor = end;
        while (editor->cursor < editor->data.count && editor->data.items[editor->cursor] == ' ') {
            editor->cursor++;
        }
    }

    SDL_free(text);
}

void evil_paste_before(Editor* editor) {
    if (!copiedLine) {
        return; // Do nothing if no line has been copied
    }

    char *text = SDL_GetClipboardText();
    if (!text) {
        fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        return;
    }

    size_t text_len = strlen(text);
    if (text_len > 0) {
        // Find the start of the current line
        size_t start = editor->cursor;
        while (start > 0 && editor->data.items[start - 1] != '\n') {
            start--;
        }

        // Insert the text from the clipboard at the start of the line
        editor_insert_buf_at(editor, text, text_len, start);

        // Optionally, insert a newline after pasting if the text doesn't end with one
        if (text[text_len - 1] != '\n') {
            editor_insert_buf_at(editor, "\n", 1, start + text_len);
        }

        // Move cursor to the first non-space character of the pasted line
        editor->cursor = start;
        while (editor->cursor < editor->data.count && editor->data.items[editor->cursor] == ' ') {
            editor->cursor++;
        }
    }

    SDL_free(text);
}

void evil_visual_char(Editor *e) {
    e->selection = true;

    // Identify the current line the cursor is on
    size_t cursor_row = editor_cursor_row(e);
    Line current_line = e->lines.items[cursor_row];

    // If in VISUAL_LINE mode, adjust the selection to span the entire line
    if (current_mode == VISUAL_LINE) {
        e->select_begin = current_line.begin;

        // Set the cursor to the end of the current line to span the whole line
        e->cursor = current_line.end;
    } else {
        e->select_begin = e->cursor;
    }
}

// TODO doesn't work
void evil_visual_line(Editor *e) {
    e->selection = true;

    // Identify the current line the cursor is on
    size_t cursor_row = editor_cursor_row(e);
    Line current_line = e->lines.items[cursor_row];

    // Set the beginning and end of the selection to span the entire line
    e->select_begin = current_line.begin;
    e->cursor = current_line.end;
}

void evil_delete_char(Editor *e) {
    if (e->searching) return;

    if (e->cursor >= e->data.count) return;

    // 1. Copy the character to clipboard.
    e->clipboard.count = 0;
    sb_append_buf(&e->clipboard, &e->data.items[e->cursor], 1);
    sb_append_null(&e->clipboard);
    if (SDL_SetClipboardText(e->clipboard.items) < 0) {
        fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
    }

    // 2. Delete the character from the editor.
    memmove(
        &e->data.items[e->cursor],
        &e->data.items[e->cursor + 1],
        (e->data.count - e->cursor - 1) * sizeof(e->data.items[0])
    );
    e->data.count -= 1;
    editor_retokenize(e);
}

void evil_delete_backward_char(Editor *e) {
    // If in search mode or at the start of the data, return.
    if (e->searching || e->cursor == 0) return;

    // Adjust the cursor to point to the previous character.
    e->cursor -= 1;

    // 1. Copy the character to clipboard.
    e->clipboard.count = 0;
    sb_append_buf(&e->clipboard, &e->data.items[e->cursor], 1);
    sb_append_null(&e->clipboard);
    if (SDL_SetClipboardText(e->clipboard.items) < 0) {
        fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
    }

    // 2. Delete the character from the editor.
    memmove(
        &e->data.items[e->cursor],
        &e->data.items[e->cursor + 1],
        (e->data.count - e->cursor - 1) * sizeof(e->data.items[0])
    );
    e->data.count -= 1;
    editor_retokenize(e);
}



void evil_search_next(Editor *e) {
    size_t startPos = e->cursor + 1;
    for (size_t pos = startPos; pos < e->data.count; ++pos) {
        if (editor_search_matches_at(e, pos)) {
            e->cursor = pos;
            editor_stop_search_and_mark(e);
            return; // Exit after finding a match
        }
    }

    // If not found in the remainder of the text, wrap around to the beginning
    for (size_t pos = 0; pos < startPos; ++pos) {
        if (editor_search_matches_at(e, pos)) {
            e->cursor = pos;
            editor_stop_search_and_mark(e);
            return; // Exit after finding a match
        }
    }
}

void evil_search_previous(Editor *e) {
    if (e->cursor == 0) {
        // If we are at the beginning of the file, wrap around immediately
        for (size_t pos = e->data.count - 1; pos != SIZE_MAX; --pos) { // Note the loop condition
            if (editor_search_matches_at(e, pos)) {
                e->cursor = pos;
                editor_stop_search_and_mark(e);
                return; // Exit after finding a match
            }
        }
    } else {
        for (size_t pos = e->cursor - 1; pos != SIZE_MAX; --pos) { // Note the loop condition
            if (editor_search_matches_at(e, pos)) {
                e->cursor = pos;
                editor_stop_search_and_mark(e);
                return; // Exit after finding a match
            }
        }

        // If not found in the preceding text, wrap around to the end
        for (size_t pos = e->data.count - 1; pos > e->cursor; --pos) {
            if (editor_search_matches_at(e, pos)) {
                e->cursor = pos;
                editor_stop_search_and_mark(e);
                return; // Exit after finding a match
            }
        }
    }
}


void evil_search_word_forward(Editor *e) {
    char word[256];

    e->searching = true;
    e->search.count = 0;

    // Extract the word under the cursor.
    if (extract_word_under_cursor(e, word)) {
        sb_append_buf(&e->search, word, strlen(word));
        editor_stop_search_and_mark(e);
        evil_search_next(e);
    } else {
        // If no word is extracted, exit search mode
        e->searching = false;
    }
}




