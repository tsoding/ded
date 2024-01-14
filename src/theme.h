#ifndef THEME_H
#define THEME_H

#include "la.h"

typedef struct {
    Vec4f cursor;
    Vec4f insert_cursor;
    Vec4f emacs_cursor;
    Vec4f text;
    Vec4f background;
    Vec4f logic;
    Vec4f comment;
    Vec4f hashtag;
    Vec4f string;
    Vec4f selection;
    Vec4f search;
    Vec4f line_numbers;
    Vec4f todo;
    Vec4f fixme;
    Vec4f note;
    Vec4f bug;
    Vec4f equals;
    Vec4f not_equals;
    Vec4f exclamation;
    Vec4f equals_equals;
    Vec4f less_than;
    Vec4f greater_than;
    Vec4f arrow;
    Vec4f plus;
    Vec4f minus;
    Vec4f truee;
    Vec4f falsee;
    Vec4f open_square;
    Vec4f close_square;
    Vec4f array_content;
    Vec4f current_line_number;
    Vec4f marks;
    Vec4f fb_selection;
    Vec4f link;
    Vec4f logic_or;
    Vec4f pipe;
    Vec4f logic_and;
    Vec4f ampersand;
    Vec4f multiplication;
    Vec4f pointer;
    Vec4f modeline;
    Vec4f modeline_accent;
    Vec4f minibuffer;
    Vec4f matching_parenthesis;
    Vec4f hl_line; 
    Vec4f type; 
    Vec4f function_definition; 
    Vec4f anchor; 
    Vec4f whitespace;
    Vec4f indentation_line;
} Theme;


extern Theme themes[];
extern int currentThemeIndex;
#define CURRENT_THEME (themes[currentThemeIndex])

void initialize_themes();
void theme_next(int *currentThemeIndex);
void theme_previous(int *currentThemeIndex);

#endif // THEME_H
