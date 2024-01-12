#ifndef EVIL_H
#define EVIL_H

#include "editor.h"

void evil_open_below(Editor *editor);
void evil_open_above(Editor *editor);
void evil_jump_item(Editor *editor);
void evil_jump_item(Editor *editor);
void evil_join(Editor *e);
void evil_yank_line(Editor *editor);
void evil_paste_after(Editor *editor);
void evil_paste_before(Editor *editor);
void evil_visual_char(Editor *e);
void evil_visual_line(Editor *e);
void evil_delete_char(Editor *e);
void evil_delete_backward_char(Editor *e);
void evil_search_next(Editor *e);
void evil_search_previous(Editor *e);
void evil_search_word_forward(Editor *e);


#endif // EVIL_H
