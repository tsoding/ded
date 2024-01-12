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
float max_zoom_factor = 10.0;

bool isAnimated = true;
bool isWave = false;
int indentation = 4;

bool showLineNumbers = false;
bool highlightCurrentLineNumber = true;
bool relativeLineNumbers = false;

bool showWhitespaces = false;
bool copiedLine = false;
bool matchParenthesis = true; //TODO segfault and highlight size

bool hl_line = false;

bool showMinibuffer = true;
bool superDrammtic = false;




// TODO bad implementation
bool extractWordUnderCursor(Editor *editor, char *word) {
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



// TODO
void move_camera(Simple_Renderer *sr, const char* direction, float amount) {
    if(sr == NULL) return;

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
Theme themes[6];

void initialize_themes() {

    // Nature
    themes[0] = (Theme) {
        .cursor = hex_to_vec4f(0x658B5FFF),
        .insert_cursor = hex_to_vec4f(0x514B8EFF),
        .emacs_cursor = hex_to_vec4f(0x834EB6FF),
        .text = hex_to_vec4f(0xC0ACD1FF),
        .background = hex_to_vec4f(0x090909FF),
        .comment = hex_to_vec4f(0x867892FF),
        .hashtag = hex_to_vec4f(0x658B5FFF),
        .logic = hex_to_vec4f(0x658B5FFF),
        .string = hex_to_vec4f(0x4C6750FF),
        .selection = hex_to_vec4f(0x262626FF),
        .search = hex_to_vec4f(0x262626FF),
        .todo = hex_to_vec4f(0x565663FF),
        .line_numbers = hex_to_vec4f(0x171717FF),
        .current_line_number = hex_to_vec4f(0xC0ACD1FF),
        .fixme = hex_to_vec4f(0x444E46FF),
        .note = hex_to_vec4f(0x4C6750FF),
        .bug = hex_to_vec4f(0x867892FF),
        .not_equals = hex_to_vec4f(0x867892FF),
        .exclamation = hex_to_vec4f(0x4C6750FF),
        .equals = hex_to_vec4f(0xC0ACD1FF),
        .equals_equals = hex_to_vec4f(0x658B5FFF),
        .greater_than = hex_to_vec4f(0x834EB6FF),
        .less_than = hex_to_vec4f(0x834EB6FF),
        .marks = hex_to_vec4f(0x565663FF),
        .fb_selection = hex_to_vec4f(0x262626FF),
        .plus = hex_to_vec4f(0x658B5FFF),
        .minus = hex_to_vec4f(0x658B5FFF),
        .truee = hex_to_vec4f(0x4C6750FF),
        .falsee = hex_to_vec4f(0x867892FF),
        .arrow = hex_to_vec4f(0x834EB6FF),
        .open_square = hex_to_vec4f(0xC0ACD1FF),
        .close_square = hex_to_vec4f(0xC0ACD1FF),
        .array_content = hex_to_vec4f(0x4C6750FF),
        .link = hex_to_vec4f(0x565663FF),
        .logic_or = hex_to_vec4f(0x658B5FFF),
        .pipe = hex_to_vec4f(0x565663FF),
        .ampersand = hex_to_vec4f(0x658B5FFF),
        .logic_and = hex_to_vec4f(0x658B5FFF),
        .pointer = hex_to_vec4f(0x514B8EFF),
        .multiplication = hex_to_vec4f(0x867892FF),
        .matching_parenthesis = hex_to_vec4f(0x262626FF),
        .hl_line = hex_to_vec4f(0x070707FF),
        .type = hex_to_vec4f(0x565663FF),
        .function_definition = hex_to_vec4f(0x564F96FF),
        .anchor = hex_to_vec4f(0x564F96FF),
        .minibuffer = hex_to_vec4f(0x090909FF),
    };

    // DOOM one
    themes[1] = (Theme) {
        .cursor = hex_to_vec4f(0x51AFEFFF), //#51AFEF
        .insert_cursor = hex_to_vec4f(0x51AFEFFF),
        .emacs_cursor = hex_to_vec4f(0xECBE7BFF), //#ECBE7B
        .text = hex_to_vec4f(0xBBC2CFFF),
        .background = hex_to_vec4f(0x282C34FF),
        .comment = hex_to_vec4f(0x5B6268FF),
        .hashtag = hex_to_vec4f(0x51AFEFFF),
        .logic = hex_to_vec4f(0x51AFEFFF),
        .string = hex_to_vec4f(0x98BE65FF), //#98BE65
        .selection = hex_to_vec4f(0x42444AFF),
        .search = hex_to_vec4f(0x387AA7FF), //#387AA7
        .todo = hex_to_vec4f(0xECBE7BFF),
        .line_numbers = hex_to_vec4f(0x3F444AFF),
        .current_line_number = hex_to_vec4f(0xBBC2CFFF),
        .fixme = hex_to_vec4f(0xFF6C6BFF), //#FF6C6B
        .note = hex_to_vec4f(0x98BE65FF),
        .bug = hex_to_vec4f(0xFF6C6BFF),
        .not_equals = hex_to_vec4f(0xFF6C6BFF),
        .exclamation = hex_to_vec4f(0x51AFEFFF),
        .equals = hex_to_vec4f(0x98BE65FF),
        .equals_equals = hex_to_vec4f(0x98BE65FF),
        .greater_than = hex_to_vec4f(0x98BE65FF),
        .less_than = hex_to_vec4f(0xFF6C6BFF),
        .marks = hex_to_vec4f(0x387AA7FF),
        .fb_selection = hex_to_vec4f(0x42444AFF),
        .plus = hex_to_vec4f(0x98BE65FF),
        .minus = hex_to_vec4f(0xFF6C6BFF),
        .truee = hex_to_vec4f(0x98BE65FF),
        .falsee = hex_to_vec4f(0xFF6C6BFF),
        .arrow = hex_to_vec4f(0xBBC2CFFF),
        .open_square = hex_to_vec4f(0xBBC2CFFF),
        .close_square = hex_to_vec4f(0xBBC2CFFF),
        .array_content = hex_to_vec4f(0xA9A1E1FF),
        .link = hex_to_vec4f(0xA9A1E1FF), //#A9A1E1
        .matching_parenthesis = hex_to_vec4f(0x42444AFF),
        .type = hex_to_vec4f(0xECBE7BFF),
        .function_definition = hex_to_vec4f(0xC678DDFF), //#C678DD
        .anchor = hex_to_vec4f(0xA9A1E1FF),
        .hl_line = hex_to_vec4f(0x21242BFF),//#21242B
        .multiplication = hex_to_vec4f(0x98BE65FF),
        .pointer = hex_to_vec4f(0xA9A1E1FF),
        .logic_and = hex_to_vec4f(0x98BE65FF),
        .logic_or = hex_to_vec4f(0xFF6C6BFF),
        .ampersand = hex_to_vec4f(0x51AFEFFF),
        .pipe = hex_to_vec4f(0x98BE65FF),
        .minibuffer = hex_to_vec4f(0x21242BFF),
    };

    // Dracula
    themes[2] = (Theme) {
        .cursor = hex_to_vec4f(0xBD93F9FF), //#BD93F9
        .insert_cursor = hex_to_vec4f(0xBD93F9FF),
        .emacs_cursor = hex_to_vec4f(0xF1FA8CFF), //#F1FA8C
        .text = hex_to_vec4f(0xF8F8F2FF),
        .background = hex_to_vec4f(0x282A36FF),
        .comment = hex_to_vec4f(0x6272A4FF),
        .hashtag = hex_to_vec4f(0xBD93F9FF),
        .logic = hex_to_vec4f(0xFF79C6FF), //#FF79C6
        .string = hex_to_vec4f(0xF1FA8CFF),
        .selection = hex_to_vec4f(0x44475AFF),
        .search = hex_to_vec4f(0x8466AEFF), //#8466AE
        .todo = hex_to_vec4f(0xF1FA8CFF),
        .line_numbers = hex_to_vec4f(0x6272A4FF),
        .current_line_number = hex_to_vec4f(0xF8F8F2FF),
        .fixme = hex_to_vec4f(0xFF5555FF), //#FF5555
        .note = hex_to_vec4f(0x50FA7BFF), //#50FA7B
        .bug = hex_to_vec4f(0xFF5555FF),
        .not_equals = hex_to_vec4f(0xFF5555FF),
        .exclamation = hex_to_vec4f(0xBD93F9FF),
        .equals = hex_to_vec4f(0x50FA7BFF),
        .equals_equals = hex_to_vec4f(0x50FA7BFF),
        .greater_than = hex_to_vec4f(0x50FA7BFF),
        .less_than = hex_to_vec4f(0xFF5555FF),
        .marks = hex_to_vec4f(0x8466AEFF),
        .fb_selection = hex_to_vec4f(0x44475AFF),
        .plus = hex_to_vec4f(0x50FA7BFF),
        .minus = hex_to_vec4f(0xFF5555FF),
        .truee = hex_to_vec4f(0x50FA7BFF),
        .falsee = hex_to_vec4f(0xFF5555FF),
        .arrow = hex_to_vec4f(0x8BE9FDFF), //#8BE9FD
        .open_square = hex_to_vec4f(0xF8F8F2FF),
        .close_square = hex_to_vec4f(0xF8F8F2FF),
        .array_content = hex_to_vec4f(0xBD93F9FF),
        .link = hex_to_vec4f(0x8BE9FDFF),
        .matching_parenthesis = hex_to_vec4f(0x44475AFF),
        .type = hex_to_vec4f(0xBD93F9FF),
        .function_definition = hex_to_vec4f(0x50FA7BFF),
        .anchor = hex_to_vec4f(0xFF79C6FF),
        .hl_line = hex_to_vec4f(0x1E2029FF), //#1E2029
        .multiplication = hex_to_vec4f(0x50FA7BFF),
        .pointer = hex_to_vec4f(0xFFC9E8FF), //#FFC9E8
        .logic_and = hex_to_vec4f(0x50FA7BFF),
        .logic_or = hex_to_vec4f(0xFF5555FF),
        .ampersand = hex_to_vec4f(0x8BE9FDFF),
        .pipe = hex_to_vec4f(0x50FA7BFF),
        .minibuffer = hex_to_vec4f(0x1E2029FF), //#1E2029
    };

    // DOOM city lights
    themes[3] = (Theme){
        .cursor = hex_to_vec4f(0x5EC4FFFF),        // #5EC4FF
        .insert_cursor = hex_to_vec4f(0xE27E8DFF), // #E27E8D
        .emacs_cursor = hex_to_vec4f(0xEBBF83FF),  // #EBBF83
        .text = hex_to_vec4f(0xA0B3C5FF),
        .background = hex_to_vec4f(0x1D252CFF),
        .comment = hex_to_vec4f(0x41505EFF),
        .hashtag = hex_to_vec4f(0x5EC4FFFF),
        .logic = hex_to_vec4f(0x5EC4FFFF),
        .string = hex_to_vec4f(0x539AFCFF), // #539AFC
        .selection = hex_to_vec4f(0x28323BFF),
        .search = hex_to_vec4f(0x4189B2FF),
        .todo = hex_to_vec4f(0xEBBF83FF),
        .line_numbers = hex_to_vec4f(0x384551FF),
        .current_line_number = hex_to_vec4f(0xA0B3C5FF),
        .fixme = hex_to_vec4f(0xD95468FF), // #D95468
        .note = hex_to_vec4f(0x8BD49CFF),  // #8BD49C
        .bug = hex_to_vec4f(0xD95468FF),
        .not_equals = hex_to_vec4f(0xD95468FF),
        .exclamation = hex_to_vec4f(0x5EC4FFFF),
        .equals = hex_to_vec4f(0x8BD49CFF),
        .equals_equals = hex_to_vec4f(0x8BD49CFF),
        .greater_than = hex_to_vec4f(0x8BD49CFF),
        .less_than = hex_to_vec4f(0xD95468FF),
        .marks = hex_to_vec4f(0x4189B2FF),
        .fb_selection = hex_to_vec4f(0x28323BFF),
        .plus = hex_to_vec4f(0x8BD49CFF),
        .minus = hex_to_vec4f(0xD95468FF),
        .truee = hex_to_vec4f(0x8BD49CFF),
        .falsee = hex_to_vec4f(0xD95468FF),
        .arrow = hex_to_vec4f(0xA0B3C5FF),
        .open_square = hex_to_vec4f(0xA0B3C5FF),
        .close_square = hex_to_vec4f(0xA0B3C5FF),
        .array_content = hex_to_vec4f(0x539AFCFF),
        .link = hex_to_vec4f(0x539AFCFF),
        .matching_parenthesis = hex_to_vec4f(0x28323BFF),
        .type = hex_to_vec4f(0xEBBF83FF),
        .function_definition = hex_to_vec4f(0x33CED8FF), // #33CED8
        .anchor = hex_to_vec4f(0xE27E8DFF),
        .hl_line = hex_to_vec4f(0x181E24FF),
        .multiplication = hex_to_vec4f(0x8BD49CFF),
        .pointer = hex_to_vec4f(0x539AFCFF),
        .logic_and = hex_to_vec4f(0x8BD49CFF),
        .logic_or = hex_to_vec4f(0xD95468FF),
        .ampersand = hex_to_vec4f(0x5EC4FFFF),
        .pipe = hex_to_vec4f(0x8BD49CFF),
        .minibuffer = hex_to_vec4f(0x181E24FF),
    };


    // DOOM molokai
    themes[4] = (Theme) {
        .cursor = hex_to_vec4f(0xFB2874FF), //#FB2874
        .insert_cursor = hex_to_vec4f(0xFB2874FF),
        .emacs_cursor = hex_to_vec4f(0xE2C770FF), //#E2C770
        .text = hex_to_vec4f(0xD6D6D4FF),
        .background = hex_to_vec4f(0x1C1E1FFF),
        .comment = hex_to_vec4f(0x555556FF),
        .hashtag = hex_to_vec4f(0x9C91E4FF), //#9C91E4
        .logic = hex_to_vec4f(0xFB2874FF),
        .string = hex_to_vec4f(0xE2C770FF),
        .selection = hex_to_vec4f(0x4E4E4EFF),
        .search = hex_to_vec4f(0x9C91E4FF),
        .todo = hex_to_vec4f(0xE2C770FF),
        .line_numbers = hex_to_vec4f(0x555556FF),
        .current_line_number = hex_to_vec4f(0xCFC0C5FF),
        .fixme = hex_to_vec4f(0xE74C3CFF), //#E74C3C
        .note = hex_to_vec4f(0xB6E63EFF), //#B6E63E
        .bug = hex_to_vec4f(0xE74C3CFF),
        .not_equals = hex_to_vec4f(0xE74C3CFF),
        .exclamation = hex_to_vec4f(0x9C91E4FF),
        .equals = hex_to_vec4f(0xB6E63EFF),
        .equals_equals = hex_to_vec4f(0xB6E63EFF),
        .greater_than = hex_to_vec4f(0xB6E63EFF),
        .less_than = hex_to_vec4f(0xE74C3CFF),
        .marks = hex_to_vec4f(0xB6E63EFF),
        .fb_selection = hex_to_vec4f(0x4E4E4EFF),
        .plus = hex_to_vec4f(0xB6E63EFF),
        .minus = hex_to_vec4f(0xE74C3CFF),
        .truee = hex_to_vec4f(0xB6E63EFF),
        .falsee = hex_to_vec4f(0xE74C3CFF),
        .arrow = hex_to_vec4f(0xD6D6D4FF),
        .open_square = hex_to_vec4f(0xD6D6D4FF),
        .close_square = hex_to_vec4f(0xD6D6D4FF),
        .array_content = hex_to_vec4f(0x9C91E4FF),
        .link = hex_to_vec4f(0x9C91E4FF),
        .matching_parenthesis = hex_to_vec4f(0x4E4E4EFF),
        .type = hex_to_vec4f(0x66D9EFFF),
        .function_definition = hex_to_vec4f(0xB6E63EFF),
        .anchor = hex_to_vec4f(0x9C91E4FF),
        .hl_line = hex_to_vec4f(0x222323FF),
        .multiplication = hex_to_vec4f(0xB6E63EFF),
        .pointer = hex_to_vec4f(0x9C91E4FF),
        .logic_and = hex_to_vec4f(0xB6E63EFF),
        .logic_or = hex_to_vec4f(0xE74C3CFF),
        .ampersand = hex_to_vec4f(0x9C91E4FF),
        .pipe = hex_to_vec4f(0xB6E63EFF),
        .minibuffer = hex_to_vec4f(0x222323FF),
    };

    

    // Palenight
    themes[5] = (Theme) {
        .cursor = hex_to_vec4f(0xC792EAFF), //#C792EA
        .insert_cursor = hex_to_vec4f(0xC792EAFF),
        .emacs_cursor = hex_to_vec4f(0xFFCB6BFF), //#FFCB6B
        .text = hex_to_vec4f(0xEEFFFFFF),
        .background = hex_to_vec4f(0x292D3EFF),
        .comment = hex_to_vec4f(0x676E95FF),
        .hashtag = hex_to_vec4f(0x89DDFFFF), //#89DDFF
        .logic = hex_to_vec4f(0x89DDFFFF),
        .string = hex_to_vec4f(0xC3E88DFF),  //#C3E88D
        .selection = hex_to_vec4f(0x3C435EFF),
        .search = hex_to_vec4f(0x4E5579FF),
        .todo = hex_to_vec4f(0xFFCB6BFF),
        .line_numbers = hex_to_vec4f(0x676E95FF),
        .current_line_number = hex_to_vec4f(0xEEFFFFFF),
        .fixme = hex_to_vec4f(0xFF5370FF), //#FF5370
        .note = hex_to_vec4f(0xC3E88DFF),
        .bug = hex_to_vec4f(0xFF5370FF),
        .not_equals = hex_to_vec4f(0xFF5370FF),
        .exclamation = hex_to_vec4f(0x89DDFFFF),
        .equals = hex_to_vec4f(0xC3E88DFF),
        .equals_equals = hex_to_vec4f(0xC3E88DFF),
        .greater_than = hex_to_vec4f(0xC3E88DFF),
        .less_than = hex_to_vec4f(0xFF5370FF),
        .marks = hex_to_vec4f(0x4E5579FF),
        .fb_selection = hex_to_vec4f(0x3C435EFF),
        .plus = hex_to_vec4f(0xC3E88DFF),
        .minus = hex_to_vec4f(0xFF5370FF),
        .truee = hex_to_vec4f(0xC3E88DFF),
        .falsee = hex_to_vec4f(0xFF5370FF),
        .arrow = hex_to_vec4f(0xFFCB6BFF),
        .open_square = hex_to_vec4f(0xEEFFFFFF),
        .close_square = hex_to_vec4f(0xEEFFFFFF),
        .array_content = hex_to_vec4f(0x82AAFFFF), //#82AAFF
        .link = hex_to_vec4f(0x89DDFFFF),
        .logic_or = hex_to_vec4f(0xFF5370FF),
        .pipe = hex_to_vec4f(0xC3E88DFF),
        .ampersand = hex_to_vec4f(0x89DDFFFF),
        .logic_and = hex_to_vec4f(0xC3E88DFF),
        .pointer = hex_to_vec4f(0xF78C6CFF), //#F78C6C
        .multiplication = hex_to_vec4f(0xC3E88DFF),
        .matching_parenthesis = hex_to_vec4f(0x3C435EFF),
        .hl_line = hex_to_vec4f(0x242837FF),
        .type = hex_to_vec4f(0xC792EAFF),
        .function_definition = hex_to_vec4f(0x82AAFFFF),
        .anchor = hex_to_vec4f(0xFF5370FF),
        .minibuffer = hex_to_vec4f(0x292D3EFF),
    };
 }

void theme_next(int *currentThemeIndex) {
    const int themeCount = sizeof(themes) / sizeof(themes[0]);
    *currentThemeIndex += 1;
    if (*currentThemeIndex >= themeCount) {
        *currentThemeIndex = 0;  // wrap around
    }
}

void theme_previous(int *currentThemeIndex) {
    *currentThemeIndex -= 1;
    if (*currentThemeIndex < 0) {
        const int themeCount = sizeof(themes) / sizeof(themes[0]);
        *currentThemeIndex = themeCount - 1;  // wrap around to the last theme
    }
}

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
        da_append(&e->data, '\0'); // Assuming da_append is a function to expand the buffer
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


void editor_kill_line(Editor *e) {
    if (e->searching || e->cursor >= e->data.count) return;

    size_t row = editor_cursor_row(e);
    size_t line_begin = e->lines.items[row].begin;
    size_t line_end = e->lines.items[row].end;

    // Check if the line is empty or if the cursor is at the end of the line
    if (line_begin == line_end || e->cursor == line_end) {
        // If the line is empty or the cursor is at the end of the line
        // Remove the newline character if it's not the first line
        if (row < e->lines.count - 1) {
            memmove(&e->data.items[line_begin], &e->data.items[line_end + 1], e->data.count - line_end - 1);
            e->data.count -= (line_end - line_begin + 1);
        } else if (row > 0 && e->data.items[line_begin - 1] == '\n') {
            // If it's the last line, remove the preceding newline character
            e->data.count -= 1;
            memmove(&e->data.items[line_begin - 1], &e->data.items[line_end], e->data.count - line_end);
        }
    } else {
        // If the line is not empty, kill the text from the cursor to the end of the line
        size_t length = line_end - e->cursor;

        // Copy the text to be killed to the clipboard
        e->clipboard.count = 0;
        sb_append_buf(&e->clipboard, &e->data.items[e->cursor], length);
        sb_append_null(&e->clipboard);
        if (SDL_SetClipboardText(e->clipboard.items) < 0) {
            fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        }

        // Delete the range from the editor
        memmove(&e->data.items[e->cursor], &e->data.items[line_end], e->data.count - line_end);
        e->data.count -= length;
    }

    editor_retokenize(e);
}


void editor_backward_kill_word(Editor *e) {
    editor_stop_search(e);

    // Remember the start position of the deletion
    size_t start_pos = e->cursor;

    // Move cursor left to the start of the previous word
    while (e->cursor > 0 && !isalnum(e->data.items[e->cursor - 1])) {
        e->cursor -= 1;
    }
    while (e->cursor > 0 && isalnum(e->data.items[e->cursor - 1])) {
        e->cursor -= 1;
    }

    // Remember the end position of the deletion
    size_t end_pos = e->cursor;

    // Check if there is anything to delete
    if (start_pos > end_pos) {
        // Copy the deleted text to clipboard
        size_t length = start_pos - end_pos;
        e->clipboard.count = 0;
        sb_append_buf(&e->clipboard, &e->data.items[end_pos], length);
        sb_append_null(&e->clipboard);
        if (SDL_SetClipboardText(e->clipboard.items) < 0) {
            fprintf(stderr, "ERROR: SDL ERROR: %s\n", SDL_GetError());
        }

        // Perform the deletion
        memmove(&e->data.items[end_pos], &e->data.items[start_pos], e->data.count - start_pos);
        e->data.count -= length;
    }

    editor_retokenize(e);
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
