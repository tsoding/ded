#include "render.h"
#include "editor.h"
#include "theme.h"

float lineNumberWidth = FREE_GLYPH_FONT_SIZE * 5;

void render_search_text(Free_Glyph_Atlas *minibuffer_atlas, Simple_Renderer *sr, Editor *editor) {
    if (editor->searching) {
        Vec4f color = themes[currentThemeIndex].text;
        Vec2f searchPos = {0.0f, 0.0f};

        simple_renderer_set_shader(sr, VERTEX_SHADER_FIXED, SHADER_FOR_TEXT);
        free_glyph_atlas_render_line_sized(minibuffer_atlas, sr, editor->search.items, editor->search.count, &searchPos, color);

        simple_renderer_flush(sr);
    }
}

void render_whitespaces(Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor) {
    if (showWhitespaces) {
        if (isWave) {
            simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_COLOR);
        } else {
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
        }
        
        float squareSize = FREE_GLYPH_FONT_SIZE * 0.2;
        float brightnessIncreasePercent = themes[currentThemeIndex].whitespace;
        
        for (size_t i = 0; i < editor->lines.count; ++i) {
            Line line = editor->lines.items[i];
            Vec2f pos = { 0, -((float)i + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE };
            
            if (showLineNumbers) {
                pos.x += lineNumberWidth;
            }
            
            for (size_t j = line.begin; j < line.end; ++j) {
                if (editor->data.items[j] == ' ' || editor->data.items[j] == '\t') {
                    Vec4f backgroundColor = themes[currentThemeIndex].background;
                    Vec4f whitespaceColor;
                    
                    // Increase each RGB component based on the percentage, but not above 1
                    whitespaceColor.x = backgroundColor.x + brightnessIncreasePercent * (1 - backgroundColor.x);
                    whitespaceColor.y = backgroundColor.y + brightnessIncreasePercent * (1 - backgroundColor.y);
                    whitespaceColor.z = backgroundColor.z + brightnessIncreasePercent * (1 - backgroundColor.z);
                    
                    // Clamp values to max 1.0
                    whitespaceColor.x = whitespaceColor.x > 1 ? 1 : whitespaceColor.x;
                    whitespaceColor.y = whitespaceColor.y > 1 ? 1 : whitespaceColor.y;
                    whitespaceColor.z = whitespaceColor.z > 1 ? 1 : whitespaceColor.z;
                    
                    // Keep the alpha value the same
                    whitespaceColor.w = backgroundColor.w;
                    
                    // Measure the actual character width
                    Vec2f char_pos = pos;
                    char_pos.x += (j - line.begin) * squareSize; // Starting position for this character
                    free_glyph_atlas_measure_line_sized(atlas, editor->data.items + j, 1, &char_pos);
                    float char_width = char_pos.x - pos.x - (j - line.begin) * squareSize;
                    
                    Vec2f rectPos = {pos.x + (j - line.begin) * char_width + (char_width - squareSize) / 2, pos.y + (FREE_GLYPH_FONT_SIZE - squareSize) / 2};
                    simple_renderer_solid_rect(sr, rectPos, vec2f(squareSize, squareSize), whitespaceColor);
                }
            }
        }
        simple_renderer_flush(sr);
    }
}





void editor_render(SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr, Editor *editor)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float max_line_len = 0.0f;

    sr->resolution = vec2f(w, h);
    sr->time = (float) SDL_GetTicks() / 1000.0f;

    /* float lineNumberWidth = FREE_GLYPH_FONT_SIZE * 5; */
    /* Vec4f lineNumberColor = vec4f(0.5, 0.5, 0.5, 1);  // A lighter color for line numbers, adjust as needed */

    // Calculate the width of a whitespace character
    Vec2f whitespace_size = vec2fs(0.0f);
    free_glyph_atlas_measure_line_sized(atlas, " ", 1, &whitespace_size);
    float whitespace_width = whitespace_size.x;


    
    // Render hl_line
    {
        if (hl_line){
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
            
            size_t currentLine = editor_cursor_row(editor);
            Vec2f highlightPos = {0.0f, -((float)currentLine + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE};
            
            float highlightWidth = 8000;  // Default width for the highlight
            
            // If showing line numbers, adjust the position and width of the highlight
            if (showLineNumbers) {
                highlightPos.x -= lineNumberWidth - 260;  // Move highlight to the left to cover line numbers
                highlightWidth += lineNumberWidth;  // Increase width to include line numbers area
            }
            
            simple_renderer_solid_rect(sr, highlightPos, vec2f(highlightWidth, FREE_GLYPH_FONT_SIZE), themes[currentThemeIndex].hl_line);
            
            simple_renderer_flush(sr);
        }
    }

    // Render anchor
    if (editor->has_anchor) {
        simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);

        // Update the anchor position before rendering
        editor_update_anchor(editor);
        
        size_t anchor_row = editor_row_from_pos(editor, editor->anchor_pos);
        Line anchor_line = editor->lines.items[anchor_row];
        size_t anchor_col = editor->anchor_pos - anchor_line.begin;
        
        Vec2f anchor_pos_vec = vec2fs(0.0f);
        anchor_pos_vec.y = -((float)anchor_row + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE;
        anchor_pos_vec.x = free_glyph_atlas_cursor_pos(
                                                       atlas,
                                                       editor->data.items + anchor_line.begin, anchor_line.end - anchor_line.begin,
                                                       vec2f(0.0, anchor_pos_vec.y),
                                                       anchor_col
                                                       );
        
        // Adjust anchor position if line numbers are shown
        if (showLineNumbers) {
            anchor_pos_vec.x += lineNumberWidth;
        }
        
        Vec4f ANCHOR_COLOR = themes[currentThemeIndex].anchor;
        
        simple_renderer_solid_rect(
                                   sr, anchor_pos_vec, vec2f(whitespace_width, FREE_GLYPH_FONT_SIZE),
                                   ANCHOR_COLOR);
        
        
        simple_renderer_flush(sr);
    }

    
    // Render selection

    {
        if (isWave){
            simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_COLOR);
        }else{
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
        }
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
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
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
        simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
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



    // Render line numbers
    if (showLineNumbers) {
        if (isWave) {
            simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_TEXT);
        } else {
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_TEXT);
        }

        // Determine the current line number using the provided function
        size_t currentLineNumber = editor_cursor_row(editor);

        // Different colors for line numbers
        Vec4f defaultColor = themes[currentThemeIndex].line_numbers;
        Vec4f currentLineColor = themes[currentThemeIndex].current_line_number;

        for (size_t i = 0; i < editor->lines.count; ++i) {
            char lineNumberStr[10];

            // Calculate display line number based on relative number setting
            size_t displayLineNumber;
            if (relativeLineNumbers) {
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
            if (highlightCurrentLineNumber && i == currentLineNumber) {
                colorToUse = currentLineColor;
            }

            free_glyph_atlas_render_line_sized(atlas, sr, lineNumberStr, strlen(lineNumberStr), &pos, colorToUse);
        }

        simple_renderer_flush(sr);
    }

    // Render matching parenthesis
    {
        if (current_mode == NORMAL || current_mode == EMACS) {
            if (matchParenthesis) {
                if (isWave) {
                    simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_COLOR);
                } else {
                    simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
                }
                
                ssize_t matching_pos = find_matching_parenthesis(editor, editor->cursor);
                if (matching_pos != -1) {
                    size_t matching_row = editor_row_from_pos(editor, matching_pos);
                    
                    Vec2f match_pos_screen = vec2fs(0.0f); // Initialize to zero
                    match_pos_screen.y = -((float)matching_row + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE;
                    
                    Line line = editor->lines.items[matching_row];
                    if (matching_pos >= line.begin && matching_pos < line.end) {
                        // Measure the position up to the matching character
                        free_glyph_atlas_measure_line_sized(atlas, editor->data.items + line.begin, matching_pos - line.begin, &match_pos_screen);
                        
                        // Measure the width of the actual character at the matching position
                        Vec2f char_end_pos = match_pos_screen;
                        free_glyph_atlas_measure_line_sized(atlas, editor->data.items + matching_pos, 1, &char_end_pos);
                        float char_width = char_end_pos.x - match_pos_screen.x;
                        
                        // Adjust for line numbers if displayed
                        if (showLineNumbers) {
                            match_pos_screen.x += lineNumberWidth;
                        }
                        
                        // Define the size of the highlight rectangle to match character size
                        Vec2f rect_size = vec2f(char_width, FREE_GLYPH_FONT_SIZE);
                        
                        simple_renderer_solid_rect(sr, match_pos_screen, rect_size, themes[currentThemeIndex].matching_parenthesis);
                    }
                }
            }
            simple_renderer_flush(sr);
        }
    }

    
    // Render text
    {
        if (isWave) {
            simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_TEXT);
        } else {
            simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_TEXT);
        }
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
                color = themes[currentThemeIndex].logic;
                break;
                
            case TOKEN_TYPE:
                color = themes[currentThemeIndex].type;
                break;
                
            case TOKEN_FUNCTION_DEFINITION:
                color = themes[currentThemeIndex].function_definition;
                break;

            case TOKEN_LINK:
                color = themes[currentThemeIndex].link;
                break;

            case TOKEN_OR:
                color = themes[currentThemeIndex].logic_or;
                break;

            case TOKEN_PIPE:
                color = themes[currentThemeIndex].pipe;
                break;

            case TOKEN_AND:
                color = themes[currentThemeIndex].logic_and;
                break;

            case TOKEN_AMPERSAND:
                color = themes[currentThemeIndex].ampersand;
                break;

            case TOKEN_POINTER:
                color = themes[currentThemeIndex].pointer;
                break;

            case TOKEN_MULTIPLICATION:
                color = themes[currentThemeIndex].multiplication;
                break;

            case TOKEN_COMMENT:
                {
                    color = themes[currentThemeIndex].comment;

                    // Checking for TODOOOO...
                    char* todoLoc = strstr(token.text, "TODO");
                    if (todoLoc && (size_t)(todoLoc - token.text + 3) < token.text_len) {

                        size_t numOs = 0;
                        char* ptr = todoLoc + 4; // Start right after "TODO"

                        // Count 'O's without crossing token boundary
                        while ((size_t)(ptr - token.text) < token.text_len && (*ptr == 'O' || *ptr == 'o')) {

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
                    if (fixmeLoc && (size_t)(fixmeLoc - token.text + 4) < token.text_len) {

                        size_t numEs = 0;
                        char* ptr = fixmeLoc + 5; // Start right after "FIXME"

                        // Count 'E's without crossing token boundary
                        while ((size_t)(ptr - token.text) < token.text_len && (*ptr == 'E' || *ptr == 'e')) {

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
                    if (bugLoc && (size_t)(bugLoc - token.text + 2) < token.text_len) {

                        color = themes[currentThemeIndex].bug;
                    }


                    // Checking for NOTE...
                    char* noteLoc = strstr(token.text, "NOTE");
                    if (noteLoc && (size_t)(noteLoc - token.text + 3) < token.text_len) {

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

    
   render_whitespaces(atlas, sr, editor);
   
    // Render minibuffer
    {
        if (showMinibuffer) {
            simple_renderer_set_shader(sr, VERTEX_SHADER_FIXED, SHADER_FOR_COLOR);
            simple_renderer_solid_rect(sr, (Vec2f){0.0f, 0.0f}, (Vec2f){w, minibufferHeight}, CURRENT_THEME.minibuffer);
            simple_renderer_flush(sr);
        }
    }


    // Render modeline
    {
        if (showModeline) {
            simple_renderer_set_shader(sr, VERTEX_SHADER_FIXED, SHADER_FOR_COLOR);
            simple_renderer_solid_rect(sr, (Vec2f){0.0f, minibufferHeight}, (Vec2f){w, modelineHeight}, CURRENT_THEME.modeline);
            // render accent
            simple_renderer_solid_rect(sr, (Vec2f){0.0f, minibufferHeight}, (Vec2f){modelineAccentWidth, modelineHeight}, CURRENT_THEME.modeline_accent);
            simple_renderer_flush(sr);
        }
    }


    
    // Render cursor
    if(editor->searching){
        simple_renderer_set_shader(sr, VERTEX_SHADER_FIXED, SHADER_FOR_COLOR);
    }else if (isWave){
        simple_renderer_set_shader(sr, VERTEX_SHADER_WAVE, SHADER_FOR_COLOR);
    }else{
        simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
    }

    {
        // Adjust cursor position if line numbers are shown
        if (showLineNumbers) {
            cursor_pos.x += lineNumberWidth;
        }

        // Constants and Default Settings
        float CURSOR_WIDTH;
        const Uint32 CURSOR_BLINK_THRESHOLD = 500;
        const Uint32 CURSOR_BLINK_PERIOD = 1000;
        const Uint32 t = SDL_GetTicks() - editor->last_stroke;
        Vec4f CURSOR_COLOR = themes[currentThemeIndex].cursor;
        float BORDER_THICKNESS = 3.0f;
        Vec4f INNER_COLOR = vec4f(CURSOR_COLOR.x, CURSOR_COLOR.y, CURSOR_COLOR.z, 0.3);

        sr->verticies_count = 0;

        // If editor has a mark, make the cursor transparent
        if (editor->has_mark) {
            CURSOR_COLOR.w = 0.0f; // Set alpha to 0 (fully transparent)
        }

        // Rendering based on mode
        switch (current_mode) {

        case NORMAL: {
            float cursor_width;
            // Check if the cursor is on an actual character or an empty line
            if (editor->cursor < editor->data.count &&
                editor->data.items[editor->cursor] != '\n') {
                    Vec2f next_char_pos = cursor_pos;
                    free_glyph_atlas_measure_line_sized(
                        atlas, editor->data.items + editor->cursor,
                        1, // Measure the actual character at the cursor
                        &next_char_pos);
                    cursor_width = next_char_pos.x - cursor_pos.x;
            } else {
                    cursor_width = whitespace_width;
            }

            simple_renderer_solid_rect(
                sr, cursor_pos, vec2f(cursor_width, FREE_GLYPH_FONT_SIZE),
                CURSOR_COLOR);
        } break;

        case EMACS: {
            float cursor_width;
            CURSOR_COLOR = themes[currentThemeIndex].emacs_cursor;
            // Check if the cursor is on an actual character or an empty line
            if (editor->cursor < editor->data.count &&
                editor->data.items[editor->cursor] != '\n') {
                Vec2f next_char_pos = cursor_pos;
                free_glyph_atlas_measure_line_sized(
                                                    atlas, editor->data.items + editor->cursor,
                                                    1, // Measure the actual character at the cursor
                                                    &next_char_pos);
                cursor_width = next_char_pos.x - cursor_pos.x;
            } else {
                cursor_width = whitespace_width;
            }
            
            // Implement blinking for EMACS mode
            if (t < CURSOR_BLINK_THRESHOLD ||
                (t / CURSOR_BLINK_PERIOD) % 2 != 0) {
                simple_renderer_solid_rect(sr, cursor_pos, vec2f(cursor_width, FREE_GLYPH_FONT_SIZE),
                                           CURSOR_COLOR);
            }
        } break;

            
        case INSERT:
            CURSOR_COLOR = themes[currentThemeIndex].insert_cursor;
            CURSOR_WIDTH = 5.0f; // Thin vertical line for INSERT mode
            // Implement blinking for INSERT mode
            if (t < CURSOR_BLINK_THRESHOLD ||
                (t / CURSOR_BLINK_PERIOD) % 2 != 0) {
                    simple_renderer_solid_rect(
                        sr, cursor_pos,
                        vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE),
                        CURSOR_COLOR);
            }
            break;

        case VISUAL: {
            float cursor_width;

            // Check if the cursor is on an actual character or an empty line
            if (editor->cursor < editor->data.count &&
                editor->data.items[editor->cursor] != '\n') {
                    Vec2f next_char_pos = cursor_pos;
                    free_glyph_atlas_measure_line_sized(
                        atlas, editor->data.items + editor->cursor, 1,
                        &next_char_pos);
                    cursor_width = next_char_pos.x - cursor_pos.x;
            } else {
                    Vec2f next_char_pos = cursor_pos;
                    free_glyph_atlas_measure_line_sized(atlas, "a", 1,
                                                        &next_char_pos);
                    cursor_width = next_char_pos.x - cursor_pos.x;
            }

            // Draw inner rectangle
            simple_renderer_solid_rect(
                sr,
                vec2f(cursor_pos.x + BORDER_THICKNESS,
                      cursor_pos.y + BORDER_THICKNESS),
                vec2f(cursor_width - 2 * BORDER_THICKNESS,
                      FREE_GLYPH_FONT_SIZE - 2 * BORDER_THICKNESS),
                INNER_COLOR);

            // Draw the outline (borders) using the theme's cursor color
            simple_renderer_solid_rect(sr, cursor_pos,
                                       vec2f(cursor_width, BORDER_THICKNESS),
                                       CURSOR_COLOR); // Top border
            simple_renderer_solid_rect(
                sr,
                vec2f(cursor_pos.x,
                      cursor_pos.y + FREE_GLYPH_FONT_SIZE - BORDER_THICKNESS),
                vec2f(cursor_width, BORDER_THICKNESS),
                CURSOR_COLOR); // Bottom border
            simple_renderer_solid_rect(
                sr, cursor_pos, vec2f(BORDER_THICKNESS, FREE_GLYPH_FONT_SIZE),
                CURSOR_COLOR); // Left border
            simple_renderer_solid_rect(
                sr,
                vec2f(cursor_pos.x + cursor_width - BORDER_THICKNESS,
                      cursor_pos.y),
                vec2f(BORDER_THICKNESS, FREE_GLYPH_FONT_SIZE),
                CURSOR_COLOR); // Right border

            break;
        }

        case VISUAL_LINE:
            // Set the cursor width to cover the entire height of the line
            CURSOR_WIDTH = FREE_GLYPH_FONT_SIZE;

            // Adjust cursor color for visual distinction. For instance, make it
            // slightly transparent
            Vec4f TRANSPARENT_CURSOR_COLOR =
                vec4f(CURSOR_COLOR.x, CURSOR_COLOR.y, CURSOR_COLOR.z,
                      0.5f); // 50% transparency

            // Render the cursor for the entire line
            simple_renderer_solid_rect(
                sr, cursor_pos, vec2f(CURSOR_WIDTH, FREE_GLYPH_FONT_SIZE),
                TRANSPARENT_CURSOR_COLOR);

            // If you'd like to add additional visual cues, consider adding a
            // border or some other distinguishing feature.
            break;
        }
        simple_renderer_flush(sr);
    }

    // Update camera
    {
        if (isAnimated) {

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