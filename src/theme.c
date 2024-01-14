#include "theme.h"
#include "common.h"

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
        .marks = hex_to_vec4f(0x658B5FFF),
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
        .modeline = hex_to_vec4f(0x060606FF),
        .modeline_accent = hex_to_vec4f(0x658B5FFF),
        .whitespace = hex_to_vec4f(0x171717FF),
        .indentation_line = hex_to_vec4f(0x171717FF),
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
        .modeline = hex_to_vec4f(0x1D2026FF),
        .modeline_accent = hex_to_vec4f(0x51AFEFFF),
        .whitespace = hex_to_vec4f(0x3F444AFF),
        .indentation_line = hex_to_vec4f(0x3F444AFF),
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
        .modeline = hex_to_vec4f(0x22232DFF),
        .modeline_accent = hex_to_vec4f(0xBD93F9FF),
        .whitespace = hex_to_vec4f(0x565761FF),
        .indentation_line = hex_to_vec4f(0x565761FF),
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
        .modeline = hex_to_vec4f(0x181F25FF),
        .modeline_accent = hex_to_vec4f(0x5EC4FFFF),
        .whitespace = hex_to_vec4f(0x384551FF),
        .indentation_line = hex_to_vec4f(0x384551FF),
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
        .modeline = hex_to_vec4f(0x2D2E2EFF),
        .modeline_accent = hex_to_vec4f(0xB6E63EFF),
        .whitespace = hex_to_vec4f(0x4E4E4EFF),
        .indentation_line = hex_to_vec4f(0x4E4E4EFF),
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
        .modeline = hex_to_vec4f(0x232635FF),
        .modeline_accent = hex_to_vec4f(0xC792EAFF),
        .whitespace = hex_to_vec4f(0x4E5579FF),
        .indentation_line = hex_to_vec4f(0x4E5579FF),
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
