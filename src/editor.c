#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "./editor.h"
#include "./common.h"
#include "./free_glyph.h"
#include "./file_browser.h"
#include <ctype.h> // For isalnum

EvilMode current_mode = NORMAL;
float zoom_factor = 5.0f;
bool showLineNumbers = false;  // This is the actual definition and initialization
bool is_animated = true;  // or false, depending on your initial requirement

bool highlight_current_line = true;
bool relative_line_numbers = true;




// TODO bad implementation
bool extractWordUnderCursor(Editor *editor, char *word) {
    // Make a copy of cursor position to avoid modifying the actual cursor
    int cursor = editor->cursor;

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

    // Debugging print
    printf("Extracted word: %s\n", word);

    return true;
}




void move_camera(Simple_Renderer *sr, const char* direction, float amount) {
    if(sr == NULL) return; // check if the SimpleRenderer pointer is valid

    // Check the direction and adjust the camera position accordingly.
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








int currentThemeIndex = 0;
Theme themes[5];

void initialize_themes() {

    themes[0] = (Theme) {
        .cursor = hex_to_vec4f(0x80D4FFFF), // Purple Cursor from --color-primary
        .text = hex_to_vec4f(0xFFFFFFFF), // Main text color, based on --color-foreground for dark mode
        .background = hex_to_vec4f(0x0D0D0DFF), // Background, from --color-background for dark mode
        .comment = hex_to_vec4f(0x808080FF), // Comments with muted foreground, from --color-foreground-muted for dark mode
        .hashtag = hex_to_vec4f(0xD6EBFFFF), // Using the hover link color for hashtags
        .logic = hex_to_vec4f(0x80D4FFFF), // Logic color same as --color-links for dark mode
        .string = hex_to_vec4f(0x2E2E2EFF), // Border color for strings
        .selection = hex_to_vec4f(0x1A1A1AFF), // Selection using the muted border color
        .search = hex_to_vec4f(0x808080FF), // Search color similar to comments
        .marks = hex_to_vec4f(0x171717FF), // Marks using the raised background
        .todo = hex_to_vec4f(0x1E1E1EFF), // Todos with slightly more visible background
        .line_numbers = hex_to_vec4f(0x808080FF), // Muted line numbers
        .fixme = hex_to_vec4f(0xCCD6F5FF), // Slightly brighter color for emphasis on FIXMEs
        .note = hex_to_vec4f(0x808080FF), // Notes with muted foreground
        .bug = hex_to_vec4f(0xF5F5F5FF), // Bugs with bright foreground
        .not_equals = hex_to_vec4f(0x80D4FFFF), // Using links color for not equals
        .exclamation = hex_to_vec4f(0xCCD6F5FF), // Slight emphasis for exclamation
        .equals = hex_to_vec4f(0x808080FF), // Muted equals color
        .equals_equals = hex_to_vec4f(0x808080FF), // Muted equals_equals color
        .greater_than = hex_to_vec4f(0x80D4FFFF), // Links color for greater_than
        .less_than = hex_to_vec4f(0x80D4FFFF), // Links color for less_than
        .plus = hex_to_vec4f(0x80D4FFFF), // Links color for plus
        .minus = hex_to_vec4f(0xCCD6F5FF), // Slight emphasis for minus
        .truee = hex_to_vec4f(0x80D4FFFF), // Links color for true
        .falsee = hex_to_vec4f(0xCCD6F5FF), // Emphasis for false
        .arrow = hex_to_vec4f(0x808080FF), // Muted arrows
        .open_square = hex_to_vec4f(0x80D4FFFF), // Links color for open brackets
        .close_square = hex_to_vec4f(0x80D4FFFF), // Links color for close brackets
        .current_line_number = hex_to_vec4f(0x80D4FFFF), // Highlight current line number with links color
        .array_content = hex_to_vec4f(0x808080FF) // Muted array content
    };

    // Base2Tone
    themes[1] = (Theme) {
        .cursor = hex_to_vec4f(0x4183c4FF), // Link Color
        .text = hex_to_vec4f(0x111111FF), // Primary Text Color
        .background = hex_to_vec4f(0x00000026), // Base Background Color
        .comment = hex_to_vec4f(0x222222FF), // Heading Colors
        .hashtag = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .logic = hex_to_vec4f(0x06f06fFF), // Button Focus Box Shadow
        .string = hex_to_vec4f(0xc0c0c0FF), // Fieldset Border Color
        .selection = hex_to_vec4f(0x999999FF), // Button Active Border Color
        .search = hex_to_vec4f(0xcccCCCFF), // Various Elements Border Color
        .todo = hex_to_vec4f(0xf8f8f8FF), // Pre Background Color
        .line_numbers = hex_to_vec4f(0x9399b2FF), // As per example
        .fixme = hex_to_vec4f(0xf7f7f7FF), // Blockquote Background
        .note = hex_to_vec4f(0x666666FF), // Blockquote Text Color
        .bug = hex_to_vec4f(0x4183c4FF), // Link Color
        .not_equals = hex_to_vec4f(0x06f06fFF), // Button Focus Box Shadow
        .exclamation = hex_to_vec4f(0x666666FF), // Blockquote Text Color
        .equals = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .equals_equals = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .greater_than = hex_to_vec4f(0x06f06fFF), // Button Focus Box Shadow
        .less_than = hex_to_vec4f(0x4183c4FF), // Link Color
        .marks = hex_to_vec4f(0x06f06fFF), // Button Focus Box Shadow
        .fb_selection = hex_to_vec4f(0xdddDDDFF), // Various Elements Border Color
        .plus = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .minus = hex_to_vec4f(0x4183c4FF), // Link Color
        .truee = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .falsee = hex_to_vec4f(0x4183c4FF), // Link Color
        .arrow = hex_to_vec4f(0x666666FF), // Blockquote Text Color
        .open_square = hex_to_vec4f(0x4183c4FF), // Link Color
        .close_square = hex_to_vec4f(0x4183c4FF), // Link Color
        .current_line_number = hex_to_vec4f(0x3ca555FF), // List Bullet Color
        .array_content = hex_to_vec4f(0xdddDDDFF), // Various Elements Border Color
    };

    // Base2Tone Extended Pink & Purple
    themes[2] = (Theme) {
        .cursor = hex_to_vec4f(0x912D56FF),            // Darker Pink for Cursor
        .text = hex_to_vec4f(0xEEEDF7FF),              // Very Light Lavender for Text
        .background = hex_to_vec4f(0x1E001380),        // Deep Purple Transparent Background
        .comment = hex_to_vec4f(0x554455FF),           // Muted Dark Purple for Comments
        .hashtag = hex_to_vec4f(0xB34688FF),           // Magenta-ish for Hashtags
        .logic = hex_to_vec4f(0xA22882FF),             // Rich Purple for Logic Operations
        .string = hex_to_vec4f(0x995D99FF),            // Purple for Strings
        .selection = hex_to_vec4f(0x4B004B80),         // Transparent Mid-Purple for Selection
        .search = hex_to_vec4f(0xDF88DFFF),            // Pink-Purple for Search Highlights
        .todo = hex_to_vec4f(0xEDE2F2FF),              // Light Lavender for TODOs
        .line_numbers = hex_to_vec4f(0x7A507AFF),      // Purple-Gray for Line Numbers
        .fixme = hex_to_vec4f(0xE8CFE8FF),             // Soft Lavender for FIXME Background
        .note = hex_to_vec4f(0x774877FF),              // Dark Lavender for Notes
        .bug = hex_to_vec4f(0xD2146BFF),               // Bright Magenta for Bugs
        .not_equals = hex_to_vec4f(0xA22882FF),        // Rich Purple for Inequality
        .exclamation = hex_to_vec4f(0xC23F91FF),       // Pink-Purple for Exclamation
        .equals = hex_to_vec4f(0x8E558E),              // Mid Purple for Equality
        .equals_equals = hex_to_vec4f(0x8E558E),       // Mid Purple for Double Equality
        .greater_than = hex_to_vec4f(0xA22882FF),      // Rich Purple for Greater Than
        .less_than = hex_to_vec4f(0x912D56FF),         // Darker Pink for Less Than
        .marks = hex_to_vec4f(0xC91C7EFF),             // Magenta for Marks
        .fb_selection = hex_to_vec4f(0xD49FD4FF),      // Muted Pink for Fallback Selections
        .plus = hex_to_vec4f(0x995D99FF),              // Purple for Addition
        .minus = hex_to_vec4f(0xB34688FF),             // Magenta-ish for Subtraction
        .truee = hex_to_vec4f(0x774877FF),             // Dark Lavender for True
        .falsee = hex_to_vec4f(0xA22882FF),            // Rich Purple for False
        .arrow = hex_to_vec4f(0x7A507AFF),             // Purple-Gray for Arrows
        .open_square = hex_to_vec4f(0x995D99FF),       // Purple for Open Brackets
        .close_square = hex_to_vec4f(0x995D99FF),      // Purple for Close Brackets
        .current_line_number = hex_to_vec4f(0xB34688FF), // Magenta-ish for Current Line Number
        .array_content = hex_to_vec4f(0xD49FD4FF)      // Muted Pink for Array Content
    };

    /* // Monokai Expanded */
    themes[3] = (Theme) {
        .cursor = hex_to_vec4f(0xF8F8F0FF),       // Off-white for Cursor
        .text = hex_to_vec4f(0xF8F8F2FF),         // Primary Text Color
        .background = hex_to_vec4f(0x272822FF),   // Base Background Color
        .comment = hex_to_vec4f(0x75715EFF),      // Grayish for Comments
        .hashtag = hex_to_vec4f(0xA6E22EFF),      // Emerald for Hashtags
        .logic = hex_to_vec4f(0xF92672FF),        // Pink for Logic Operations
        .string = hex_to_vec4f(0xE6DB74FF),       // Yellow for Strings
        .selection = hex_to_vec4f(0x49483EFF),    // Darker Gray for Selection
        .search = hex_to_vec4f(0x66D9EFFF),       // Cyan for Search Highlights
        .todo = hex_to_vec4f(0xFD971FFF),         // Orange-red for TODO markers
        .line_numbers = hex_to_vec4f(0x8F908AFF),// Muted Gray for Line Numbers
        .fixme = hex_to_vec4f(0xFF0000FF),        // Red for FIXME markers
        .note = hex_to_vec4f(0x66D9EFFF),         // Cyan for Notes
        .bug = hex_to_vec4f(0xFD5F00FF),          // Orange for Bugs
        .not_equals = hex_to_vec4f(0xA6E22EFF),   // Emerald for Inequality
        .exclamation = hex_to_vec4f(0xF92672FF),  // Pink for Exclamation
        .equals = hex_to_vec4f(0x66D9EFFF),       // Cyan for Equality
        .greater_than = hex_to_vec4f(0xF92672FF), // Pink for Comparison Operators
        .less_than = hex_to_vec4f(0xA6E22EFF),    // Emerald for Comparison Operators
        .marks = hex_to_vec4f(0xFD971FFF),        // Orange-red for Marks
        .fb_selection = hex_to_vec4f(0x3E3D32FF), // Darker Shade for Fallback Selections
        .plus = hex_to_vec4f(0x66D9EFFF),         // Cyan for Addition
        .minus = hex_to_vec4f(0xF92672FF),        // Pink for Subtraction
        .truee = hex_to_vec4f(0xA6E22EFF),        // Emerald for True
        .falsee = hex_to_vec4f(0xF92672FF),       // Pink for False
        .arrow = hex_to_vec4f(0xE6DB74FF),        // Yellow for Arrows
        .open_square = hex_to_vec4f(0xE6DB74FF),  // Yellow for Open Brackets
        .close_square = hex_to_vec4f(0xE6DB74FF), // Yellow for Close Brackets
        .current_line_number = hex_to_vec4f(0x66D9EFFF), // Cyan for Current Line
        .array_content = hex_to_vec4f(0x3E3D32FF) // Darker Shade for Array Content
    };

    // Catppuccin
    themes[4] = (Theme) {
        .cursor = hex_to_vec4f(0xf38ba8FF), // Red
        .text = hex_to_vec4f(0xcdd6f4FF), // Text
        .background = hex_to_vec4f(0x1e1e2eFF), // Base
        .comment = hex_to_vec4f(0x9399b2FF), // Overlay2
        .hashtag = hex_to_vec4f(0x89b4faFF), // Blue
        .logic = hex_to_vec4f(0xCBA6F7FF), // Peach
        .string = hex_to_vec4f(0xf9e2afFF), // Yellow
        .selection = hex_to_vec4f(0xf5c2e7FF), // Pink
        .search = hex_to_vec4f(0xf2cdcdFF), // Flamingo
        .todo = hex_to_vec4f(0xf2cdcdFF), // Flamingo
        .line_numbers = hex_to_vec4f(0x9399b2FF), // Overlay2
        .fixme = hex_to_vec4f(0xf2cdcdFF), // Flamingo
        .note = hex_to_vec4f(0xa6e3a1FF), // Green
        .bug = hex_to_vec4f(0xf38ba8FF), // Red
        .not_equals = hex_to_vec4f(0xf38ba8FF), // Red
        .exclamation = hex_to_vec4f(0xf38ba8FF), // Red
        .equals = hex_to_vec4f(0xa6e3a1FF), // Green
        .equals_equals = hex_to_vec4f(0xa6e3a1FF), // Green
        .greater_than = hex_to_vec4f(0xa6e3a1FF), // Green
        .less_than = hex_to_vec4f(0x74c7ecFF), // Sapphire
        .marks = hex_to_vec4f(0x74c7ecFF), // Sapphire
        .fb_selection = hex_to_vec4f(0xb4befeFF), // Lavender
        .plus = hex_to_vec4f(0xa6e3a1FF), // Green
        .minus = hex_to_vec4f(0xf38ba8FF), // Red
        .truee = hex_to_vec4f(0xa6e3a1FF), // Green
        .falsee = hex_to_vec4f(0xf38ba8FF), // Red
        .arrow = hex_to_vec4f(0xf9e2afFF), // Yellow
        .open_square = hex_to_vec4f(0x89b4faFF), // Blue
        .close_square = hex_to_vec4f(0x89b4faFF), // Blue
        .current_line_number = hex_to_vec4f(0x89b4faFF), // Blue
        .array_content = hex_to_vec4f(0x74c7ecFF), // Sapphire
    };
 }

void theme_next(int *currentThemeIndex) {
    const int themeCount = sizeof(themes) / sizeof(themes[0]);
    *currentThemeIndex += 1;
    if (*currentThemeIndex >= themeCount) {
        *currentThemeIndex = 0;  // wrap around if we've gone past the last theme
    }
}

void theme_previous(int *currentThemeIndex) {
    *currentThemeIndex -= 1;
    if (*currentThemeIndex < 0) {
        const int themeCount = sizeof(themes) / sizeof(themes[0]);
        *currentThemeIndex = themeCount - 1;  // wrap around to the last theme
    }
}

// Smart Parenthesis
void editor_backspace(Editor *e)
{
    if (e->searching) {
        if (e->search.count > 0) {
            e->search.count -= 1;
        }
    } else {
        if (e->cursor == 0) return; // Cursor at the beginning, nothing to delete

        size_t cursor_pos = e->cursor;

        if (cursor_pos > e->data.count) {
            cursor_pos = e->data.count;
        }

        // Determine the characters before and after the cursor
        char char_before_cursor = (cursor_pos > 0) ? e->data.items[cursor_pos - 1] : '\0';
        char char_after_cursor = (cursor_pos < e->data.count) ? e->data.items[cursor_pos] : '\0';

        if ((char_before_cursor == '(' && char_after_cursor == ')') ||
            (char_before_cursor == '[' && char_after_cursor == ']') ||
            (char_before_cursor == '{' && char_after_cursor == '}') ||
            (char_before_cursor == '\'' && char_after_cursor == '\'') ||
            (char_before_cursor == '"' && char_after_cursor == '"')) {
            // Delete both characters and move cursor left
            memmove(&e->data.items[cursor_pos - 1], &e->data.items[cursor_pos + 1], e->data.count - cursor_pos);
            e->cursor -= 1;
            e->data.count -= 2;
        } else {
            // Delete only the character before the cursor
            memmove(&e->data.items[cursor_pos - 1], &e->data.items[cursor_pos], e->data.count - cursor_pos);
            e->cursor -= 1;
            e->data.count -= 1;
        }

        editor_retokenize(e);
    }
}

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

    if (e->cursor > e->select_begin) {
        if (e->cursor > e->data.count) {
            e->cursor = e->data.count;
        }
        if (e->cursor == 0) return;

        size_t nchars = e->cursor - e->select_begin;
        memmove(
            &e->data.items[e->cursor - nchars],
            &e->data.items[e->cursor],
            e->data.count - e->cursor
        );

        e->cursor -= nchars;
        e->data.count -= nchars;
    } else {
        if (e->cursor >= e->data.count) return;

        size_t nchars = e->select_begin - e->cursor;
        memmove(
            &e->data.items[e->cursor],
            &e->data.items[e->cursor + nchars],
            e->data.count - e->cursor - nchars
        );

        e->data.count -= nchars;
    }
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

void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float max_line_len = 0.0f;

    sr->resolution = vec2f(w, h);
    sr->time = (float) SDL_GetTicks() / 1000.0f;

    float lineNumberWidth = FREE_GLYPH_FONT_SIZE * 5;
    /* Vec4f lineNumberColor = vec4f(0.5, 0.5, 0.5, 1);  // A lighter color for line numbers, adjust as needed */




    
    // Render selection
    {
        simple_renderer_set_shader(sr, SHADER_FOR_COLOR);
        if (editor->selection) {
            for (size_t row = 0; row < editor->lines.count; ++row) {
                size_t select_begin_chr = editor->select_begin;
                size_t select_end_chr = editor->cursor;
                if (select_begin_chr > select_end_chr) {
                    SWAP(size_t, select_begin_chr, select_end_chr);
                }

                Line line_chr = editor->lines.items[row];

                if (select_begin_chr < line_chr.begin) {
                    select_begin_chr = line_chr.begin;
                }

                if (select_end_chr > line_chr.end) {
                    select_end_chr = line_chr.end;
                }

                if (select_begin_chr <= select_end_chr) {
                    Vec2f select_begin_scr = vec2f(0, -((float)row + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE);
                    free_glyph_atlas_measure_line_sized(
                                                        atlas, editor->data.items + line_chr.begin, select_begin_chr - line_chr.begin,
                                                        &select_begin_scr);

                    Vec2f select_end_scr = select_begin_scr;
                    free_glyph_atlas_measure_line_sized(
                                                        atlas, editor->data.items + select_begin_chr, select_end_chr - select_begin_chr,
                                                        &select_end_scr);

                    // Adjust selection for line numbers if displayed
                    if (showLineNumbers) {
                        select_begin_scr.x += lineNumberWidth;
                        select_end_scr.x += lineNumberWidth;
                    }

                    Vec4f selection_color = vec4f(.25, .25, .25, 1);

                    simple_renderer_solid_rect(sr, select_begin_scr, vec2f(select_end_scr.x - select_begin_scr.x, FREE_GLYPH_FONT_SIZE), selection_color);
                }
            }
        }
        simple_renderer_flush(sr);
    }

    Vec2f cursor_pos = vec2fs(0.0f);
    {
        size_t cursor_row = editor_cursor_row(editor);
        Line line = editor->lines.items[cursor_row];
        size_t cursor_col = editor->cursor - line.begin;
        cursor_pos.y = -((float)cursor_row + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE;
        cursor_pos.x = free_glyph_atlas_cursor_pos(
                           atlas,
                           editor->data.items + line.begin, line.end - line.begin,
                           vec2f(0.0, cursor_pos.y),
                           cursor_col
                       );
    }

    // Render search
    {
        if (editor->searching) {
            simple_renderer_set_shader(sr, SHADER_FOR_COLOR);
            Vec4f selection_color = themes[currentThemeIndex].search; // or .selection_color if that's what you named it in the struct.

            Vec2f p1 = cursor_pos;
            Vec2f p2 = p1;

            free_glyph_atlas_measure_line_sized(editor->atlas, editor->search.items, editor->search.count, &p2);

            // Adjust for line numbers width if they are displayed
            if (showLineNumbers) {
                p1.x += lineNumberWidth;
                p2.x += lineNumberWidth;
            }

            simple_renderer_solid_rect(sr, p1, vec2f(p2.x - p1.x, FREE_GLYPH_FONT_SIZE), selection_color);
            simple_renderer_flush(sr);
        }
    }

    // Render marked search result
    {
        simple_renderer_set_shader(sr, SHADER_FOR_COLOR);
        if (editor->has_mark) {
            for (size_t row = 0; row < editor->lines.count; ++row) {
                size_t mark_begin_chr = editor->mark_start;
                size_t mark_end_chr = editor->mark_end;

                Line line_chr = editor->lines.items[row];

                if (mark_begin_chr < line_chr.begin) {
                    mark_begin_chr = line_chr.begin;
                }

                if (mark_end_chr > line_chr.end) {
                    mark_end_chr = line_chr.end;
                }

                if (mark_begin_chr <= mark_end_chr) {
                    Vec2f mark_begin_scr = vec2f(0, -((float)row + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE);
                    free_glyph_atlas_measure_line_sized(
                                                        atlas, editor->data.items + line_chr.begin, mark_begin_chr - line_chr.begin,
                                                        &mark_begin_scr);

                    Vec2f mark_end_scr = mark_begin_scr;
                    free_glyph_atlas_measure_line_sized(
                                                        atlas, editor->data.items + mark_begin_chr, mark_end_chr - mark_begin_chr,
                                                        &mark_end_scr);

                    // Adjust for line numbers width if they are displayed
                    if (showLineNumbers) {
                        mark_begin_scr.x += lineNumberWidth;
                        mark_end_scr.x += lineNumberWidth;
                    }

                    Vec4f mark_color = themes[currentThemeIndex].marks;
                    simple_renderer_solid_rect(sr, mark_begin_scr, vec2f(mark_end_scr.x - mark_begin_scr.x, FREE_GLYPH_FONT_SIZE), mark_color);
                }
            }
        }
        simple_renderer_flush(sr);
    }


    /* current line number */
    if (showLineNumbers) {
        simple_renderer_set_shader(sr, SHADER_FOR_TEXT);

        // Determine the current line number using the provided function
        size_t currentLineNumber = editor_cursor_row(editor);

        // Different colors for line numbers
        Vec4f defaultColor = themes[currentThemeIndex].line_numbers;
        Vec4f currentLineColor = themes[currentThemeIndex].current_line_number;

        for (size_t i = 0; i < editor->lines.count; ++i) {
            char lineNumberStr[10];

            // Calculate display line number based on relative number setting
            size_t displayLineNumber;
            if (relative_line_numbers) {
                if (i == currentLineNumber) {
                    // Display the actual line number for the current line
                    displayLineNumber = currentLineNumber + 1;
                } else {
                    // Show the distance from the current line for other lines
                    displayLineNumber = (i > currentLineNumber) ? i - currentLineNumber : currentLineNumber - i;
                }
            } else {
                displayLineNumber = i + 1;
            }

            snprintf(lineNumberStr, sizeof(lineNumberStr), "%zu", displayLineNumber);

            Vec2f pos = {0, -((float)i + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE};

            // Decide on the color to use
            Vec4f colorToUse = defaultColor;
            if (highlight_current_line && i == currentLineNumber) {
                colorToUse = currentLineColor;
            }

            free_glyph_atlas_render_line_sized(atlas, sr, lineNumberStr, strlen(lineNumberStr), &pos, colorToUse);
        }

        simple_renderer_flush(sr);
    }


    // Render text
    {
        simple_renderer_set_shader(sr, SHADER_FOR_TEXT);
        for (size_t i = 0; i < editor->tokens.count; ++i) {
            Token token = editor->tokens.items[i];
            Vec2f pos = token.position;
            //Vec4f color = vec4fs(1);
            // TODO match color for open and close
            Vec4f color = themes[currentThemeIndex].text;

            // Adjust for line numbers width if they are displayed
            if (showLineNumbers) {
                pos.x += lineNumberWidth;
            }

            switch (token.kind) {
            case TOKEN_PREPROC:
                if (token.text_len >= 7 && token.text[0] == '#') { // Check if it's likely a hex color
                    bool valid_hex = true;
                    for (size_t j = 1; j < 7 && valid_hex; ++j) {
                        if (!is_hex_digit(token.text[j])) {
                            valid_hex = false;
                        }
                    }

                    if (valid_hex) {
                        unsigned int hex_value;
                        if(sscanf(token.text, "#%06x", &hex_value) == 1) {
                            color = hex_to_vec4f(hex_value);
                        } else {
                            color = themes[currentThemeIndex].hashtag; // Default to the hashtag color if not a valid hex
                        }
                    } else {
                        color = themes[currentThemeIndex].hashtag; // Not a valid hex color
                    }
                } else {
                    color = themes[currentThemeIndex].hashtag; // Default color for preprocessor directives
                }
                break;

            case TOKEN_KEYWORD:
                /* color = hex_to_vec4f(0xFFDD33FF); */
                color = themes[currentThemeIndex].logic;
                break;

            case TOKEN_COMMENT:
                {
                    color = themes[currentThemeIndex].comment;

                    // Checking for TODOOOO...
                    char* todoLoc = strstr(token.text, "TODO");
                    if (todoLoc && (todoLoc - token.text + 3) < token.text_len) {
                        size_t numOs = 0;
                        char* ptr = todoLoc + 4; // Start right after "TODO"

                        // Count 'O's without crossing token boundary
                        while ((ptr - token.text) < token.text_len && (*ptr == 'O' || *ptr == 'o')) {
                            numOs++;
                            ptr++;
                        }

                        Vec4f baseColor = themes[currentThemeIndex].todo;
                        float deltaRed = (1.0f - baseColor.x) / 5;  // Adjusting for maximum of TODOOOOO

                        color.x = baseColor.x + deltaRed * numOs;
                        color.y = baseColor.y * (1 - 0.2 * numOs);
                        color.z = baseColor.z * (1 - 0.2 * numOs);
                        color.w = baseColor.w;
                    }

                    // Checking for FIXMEEEE...
                    char* fixmeLoc = strstr(token.text, "FIXME");
                    if (fixmeLoc && (fixmeLoc - token.text + 4) < token.text_len) {
                        size_t numEs = 0;
                        char* ptr = fixmeLoc + 5; // Start right after "FIXME"

                        // Count 'E's without crossing token boundary
                        while ((ptr - token.text) < token.text_len && (*ptr == 'E' || *ptr == 'e')) {
                            numEs++;
                            ptr++;
                        }

                        Vec4f baseColor = themes[currentThemeIndex].fixme;
                        float deltaRed = (1.0f - baseColor.x) / 5;  // Adjusting for maximum of FIXMEEEE

                        color.x = baseColor.x + deltaRed * numEs;
                        color.y = baseColor.y * (1 - 0.2 * numEs);
                        color.z = baseColor.z * (1 - 0.2 * numEs);
                        color.w = baseColor.w;
                    }

                    // Checking for BUG...
                    char* bugLoc = strstr(token.text, "BUG");
                    if (bugLoc && (bugLoc - token.text + 2) < token.text_len) {
                        color = themes[currentThemeIndex].bug;
                    }


                    // Checking for NOTE...
                    char* noteLoc = strstr(token.text, "NOTE");
                    if (noteLoc && (noteLoc - token.text + 3) < token.text_len) {
                        color = themes[currentThemeIndex].note;
                    }

                    // Continue rendering with
                }
                break;


            case TOKEN_EQUALS:
                color = themes[currentThemeIndex].equals;
                break;

            case TOKEN_EXCLAMATION:
                color = themes[currentThemeIndex].exclamation;
                break;

            case TOKEN_NOT_EQUALS:
                color = themes[currentThemeIndex].not_equals;
                break;

            case TOKEN_EQUALS_EQUALS:
                color = themes[currentThemeIndex].equals_equals;
                break;


            case TOKEN_LESS_THAN:
                color = themes[currentThemeIndex].less_than;
                break;

            case TOKEN_GREATER_THAN:
                color = themes[currentThemeIndex].greater_than;
                break;
            case TOKEN_ARROW:
                color = themes[currentThemeIndex].arrow;
                break;

            case TOKEN_MINUS:
                color = themes[currentThemeIndex].minus;
                break;

            case TOKEN_PLUS:
                color = themes[currentThemeIndex].plus;
                break;

            case TOKEN_TRUE:
                color = themes[currentThemeIndex].truee;
                break;
            case TOKEN_FALSE:
                color = themes[currentThemeIndex].falsee;
                break;
            case TOKEN_OPEN_SQUARE:
                color = themes[currentThemeIndex].open_square;
                break;
            case TOKEN_CLOSE_SQUARE:
                color = themes[currentThemeIndex].close_square;
                break;
            case TOKEN_ARRAY_CONTENT:
                color = themes[currentThemeIndex].array_content;
                break;
            case TOKEN_BAD_SPELLCHECK:
                color = themes[currentThemeIndex].bug;
                break;
            case TOKEN_STRING:
                /* color = hex_to_vec4f(0x73c936ff); */
                color = themes[currentThemeIndex].string;
                break;
            case TOKEN_COLOR: // Added case for TOKEN_COLOR
                {
                    unsigned long long hex_value;
                    if(sscanf(token.text, "0x%llx", &hex_value) == 1) {
                        color = hex_to_vec4f((uint32_t)hex_value);
                    }
                }
                break;
            default:
                {}
            }
            free_glyph_atlas_render_line_sized(atlas, sr, token.text, token.text_len, &pos, color);
            // TODO: the max_line_len should be calculated based on what's visible on the screen right now
            if (max_line_len < pos.x) max_line_len = pos.x;
        }
        simple_renderer_flush(sr);
    }


    // Render cursor
    simple_renderer_set_shader(sr, SHADER_FOR_EPICNESS);

    // Exit early if the editor has a mark and should not render the cursor
    // since the camera follow the cursor i cant do it or i dont know how
    /* if (editor->has_mark) { */
    /*     return;  // Skip the cursor rendering */
    /* } */

    // Adjust cursor position if line numbers are shown
    if (showLineNumbers) {
        cursor_pos.x += lineNumberWidth;
    }

    // Constants and Default Settings
    float CURSOR_WIDTH;
    const Uint32 CURSOR_BLINK_THRESHOLD = 500;
    const Uint32 CURSOR_BLINK_PERIOD = 1000;
    const Uint32 t = SDL_GetTicks() - editor->last_stroke;
    Vec4f CURSOR_COLOR = themes[currentThemeIndex].cursor;  // Default cursor color
    float VISUAL_CURSOR_WIDTH = FREE_GLYPH_FONT_SIZE / 2.0f;
    float BORDER_THICKNESS = 5.0f;
    Vec4f INNER_COLOR = vec4f(CURSOR_COLOR.x, CURSOR_COLOR.y, CURSOR_COLOR.z, 0.3); // Same color but with reduced alpha

    sr->verticies_count = 0;

    // Rendering based on mode
    switch (current_mode) {
    case NORMAL:
        CURSOR_WIDTH = FREE_GLYPH_FONT_SIZE / 2.0f; // Half the size for NORMAL mode
        simple_renderer_solid_rect(sr, cursor_pos, vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE), CURSOR_COLOR);
        break;

    case INSERT:
        CURSOR_WIDTH = 5.0f; // Thin vertical line for INSERT mode
        // Implement blinking for INSERT mode
        if (t < CURSOR_BLINK_THRESHOLD || (t / CURSOR_BLINK_PERIOD) % 2 != 0) {
            simple_renderer_solid_rect(sr, cursor_pos, vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE), CURSOR_COLOR);
        }
        break;

    case VISUAL:
        // Draw inner rectangle with reduced alpha
        simple_renderer_solid_rect(sr, cursor_pos, vec2f(VISUAL_CURSOR_WIDTH - 2 * BORDER_THICKNESS, FREE_GLYPH_FONT_SIZE - 2 * BORDER_THICKNESS), INNER_COLOR);

        // Draw the outline (borders) using the theme's cursor color
        // Top border
        simple_renderer_solid_rect(sr, cursor_pos, vec2f(VISUAL_CURSOR_WIDTH, BORDER_THICKNESS), CURSOR_COLOR);

        // Bottom border
        simple_renderer_solid_rect(sr, vec2f(cursor_pos.x, cursor_pos.y + FREE_GLYPH_FONT_SIZE - BORDER_THICKNESS), vec2f(VISUAL_CURSOR_WIDTH, BORDER_THICKNESS), CURSOR_COLOR);

        // Left border
        simple_renderer_solid_rect(sr, cursor_pos, vec2f(BORDER_THICKNESS, FREE_GLYPH_FONT_SIZE), CURSOR_COLOR);

        // Right border
        simple_renderer_solid_rect(sr, vec2f(cursor_pos.x + VISUAL_CURSOR_WIDTH - BORDER_THICKNESS, cursor_pos.y), vec2f(BORDER_THICKNESS, FREE_GLYPH_FONT_SIZE), CURSOR_COLOR);

        break;
    }

    // Update camera
    {
        if (is_animated) {
            // Your current camera update logic for animated behavior

            if (max_line_len > 1000.0f) {
                max_line_len = 1000.0f;
            }

            float target_scale = w / zoom_factor / (max_line_len * 0.75); // TODO: division by 0

            Vec2f target = cursor_pos;
            float offset = 0.0f;

            if (target_scale > 3.0f) {
                target_scale = 3.0f;
            } else {
                offset = cursor_pos.x - w/3/sr->camera_scale;
                if (offset < 0.0f) offset = 0.0f;
                target = vec2f(w/3/sr->camera_scale + offset, cursor_pos.y);
            }

            sr->camera_vel = vec2f_mul(
                                       vec2f_sub(target, sr->camera_pos),
                                       vec2fs(2.0f));
            sr->camera_scale_vel = (target_scale - sr->camera_scale) * 2.0f;

            sr->camera_pos = vec2f_add(sr->camera_pos, vec2f_mul(sr->camera_vel, vec2fs(DELTA_TIME)));
            sr->camera_scale = sr->camera_scale + sr->camera_scale_vel * DELTA_TIME;

        } else {
            sr->camera_scale = 0.24f;  // Set the zoom level to 0.24

            // Static flag to ensure initial camera position is set only once
            static bool hasSetInitialPosition = false;

            // If the initial position hasn't been set, set it now
            if (!hasSetInitialPosition) {
                sr->camera_pos.x = 3850.0f;  // Set the x-position
                sr->camera_pos.y = -2000.0f;  // Set the initial y-position
                hasSetInitialPosition = true;
            } else {
                // Calculate the vertical position of the cursor in world coordinates.
                int currentLine = editor_cursor_row(editor);
                float cursorPosY = -((float)currentLine + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE;

                // Define the top and bottom edges of the current camera view.
                float cameraTopEdge = sr->camera_pos.y - (h/2.0f) / sr->camera_scale;
                float cameraBottomEdge = sr->camera_pos.y + (h/2.0f) / sr->camera_scale;

                // Adjust the camera's Y position if the cursor is outside the viewport.
                if (cursorPosY > cameraBottomEdge) {
                    sr->camera_pos.y += cursorPosY - cameraBottomEdge;  // Move camera down just enough
                } else if (cursorPosY < cameraTopEdge) {
                    sr->camera_pos.y -= cameraTopEdge - cursorPosY;  // Move camera up just enough
                }

                // Keeping the x-position fixed as per the previous logic
                sr->camera_pos.x = 3850.0f;
            }
        }
    }
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

// ADDED
void editor_cut_char_under_cursor(Editor *e) {
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

// VISUAL selection

void editor_start_visual_selection(Editor *e) {
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

void editor_start_visual_line_selection(Editor *e) {
    e->selection = true;

    // Identify the current line the cursor is on
    size_t cursor_row = editor_cursor_row(e);
    Line current_line = e->lines.items[cursor_row];

    // Set the beginning and end of the selection to span the entire line
    e->select_begin = current_line.begin;
    e->cursor = current_line.end;
}




void editor_update_selection(Editor *e, bool shift) {
    if (e->searching) return;

    if (current_mode == VISUAL) {
        if (!e->selection) {
            editor_start_visual_selection(e);
        }
        // If you want the selection to end when you leave VISUAL mode,
        // you will need to handle that logic elsewhere (perhaps where mode changes are managed).
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

void editor_search_next(Editor *e) {
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

void editor_search_previous(Editor *e) {
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
