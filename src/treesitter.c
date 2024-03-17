#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <tree_sitter/api.h>
#include "theme.h"
#include "treesitter.h"

// Declare the `tree_sitter_json` function, which is
// implemented by the `tree-sitter-json` library.
TSLanguage *tree_sitter_json();


void tree(Editor *e, File_Browser *fb) {
    // Create a parser.
    TSParser *parser = ts_parser_new();
        
    // Set the parser's language (JSON in this case).
    ts_parser_set_language(parser, tree_sitter_json());


    // Build a syntax tree based on the editor data.
    TSTree *tree = ts_parser_parse_string(
                                          parser,
                                          NULL,
                                          e->data.items,
                                          strlen(e->data.items)
                                          );

    // Get the root node of the syntax tree
    TSNode root_node = ts_tree_root_node(tree);

    // Update theme colors based on syntax
    update_theme_colors_based_on_syntax(root_node, &CURRENT_THEME);
    
    // Get some child nodes.
    TSNode array_node = ts_node_named_child(root_node, 0);
    TSNode number_node = ts_node_named_child(array_node, 0);

    /* // Check that the nodes have the expected types. */
    /* assert(strcmp(ts_node_type(root_node), "document") == 0); */
    /* assert(strcmp(ts_node_type(array_node), "array") == 0); */
    /* assert(strcmp(ts_node_type(number_node), "number") == 0); */

    /* // Check that the nodes have the expected child counts. */
    /* assert(ts_node_child_count(root_node) == 1); */
    /* assert(ts_node_child_count(array_node) == 5); */
    /* assert(ts_node_named_child_count(array_node) == 2); */
    /* assert(ts_node_child_count(number_node) == 0); */

    // Print the syntax tree as an S-expression.
    char *string = ts_node_string(root_node);
    printf("Syntax tree: %s\n", string);

    // Free all of the heap-allocated memory.
    free(string);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
}





void update_theme_colors_based_on_syntax(TSNode node, Theme *theme) {
    if (ts_node_is_null(node)) return;

    const char* node_type = ts_node_type(node);

    // Change theme colors based on node type
    if (strcmp(node_type, "string") == 0) {
        transition_color(&theme->string, theme->bug, 0.005f);
    } else if (strcmp(node_type, "comment") == 0) {
        transition_color(&theme->comment, theme->bug, 0.005f);
    } else if (strcmp(node_type, "keyword") == 0) {
        transition_color(&theme->logic, theme->bug, 0.005f);
    }
    // ... add more types as needed

    // Recursively update theme colors for child nodes
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        update_theme_colors_based_on_syntax(child, theme);
    }
}
