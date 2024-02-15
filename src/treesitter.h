
#ifndef TREESITTER_H
#define TREESITTER_H

#include "editor.h"
#include "file_browser.h"
#include "theme.h"
#include <tree_sitter/api.h>


void tree(Editor *e, File_Browser *fb);
void apply_syntax_highlighting(Editor *editor, TSNode node);
void highlight_node(Editor *editor, TSNode node, Vec4f color);

void update_theme_colors_based_on_syntax(TSNode node, Theme *theme);

#endif // TREESITTER_H



