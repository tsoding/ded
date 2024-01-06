// YANSIPPET

#include <dirent.h>
#include "editor.h"
#include <errno.h>
#include "yasnippet.h"


SnippetArray snippets;

void init_snippet_array(SnippetArray *a, size_t initial_size) {
    a->array = (Snippet *)malloc(initial_size * sizeof(Snippet));
    a->used = 0;
    a->size = initial_size;
}

void insert_snippet(SnippetArray *a, Snippet snippet) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (Snippet *)realloc(a->array, a->size * sizeof(Snippet));
    }
    a->array[a->used++] = snippet;
}

void free_snippet_array(SnippetArray *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}


void load_snippets_from_directory() {
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "ERROR: HOME environment variable not set.\n");
        return;
    }

    char directory[256];
    snprintf(directory, sizeof(directory), "%s/.config/ded/snippets", home);

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(directory)) == NULL) {
        fprintf(stderr, "opendir failed: %s\n", strerror(errno));
        return;
    }

    init_snippet_array(&snippets, 10); // Start with an initial size of 10

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);

            FILE *file = fopen(filepath, "r");
            if (file) {
                Snippet new_snippet;
                strncpy(new_snippet.key, entry->d_name, MAX_SNIPPET_KEY_LENGTH - 1);
                new_snippet.key[MAX_SNIPPET_KEY_LENGTH - 1] = '\0';
                new_snippet.content[0] = '\0'; // Initialize content as empty string

                char line[256];
                while (fgets(line, sizeof(line), file)) {
                    strncat(new_snippet.content, line, MAX_SNIPPET_CONTENT_LENGTH - strlen(new_snippet.content) - 1);
                }
                new_snippet.content[MAX_SNIPPET_CONTENT_LENGTH - 1] = '\0';

                insert_snippet(&snippets, new_snippet);
                fclose(file);
            }
        }
    }

    closedir(dir);
}



bool get_word_left_of_cursor(Editor *e, char *word, size_t max_word_length) {
    if (e->cursor == 0 || !isalnum(e->data.items[e->cursor - 1])) {
        return false; // No word directly to the left of the cursor
    }

    size_t end = e->cursor;
    size_t start = end;

    while (start > 0 && isalnum(e->data.items[start - 1])) {
        start--;
    }

    size_t word_length = end - start;
    if (word_length >= max_word_length) {
        return false; // Word is too long for the buffer
    }

    memcpy(word, &e->data.items[start], word_length);
    word[word_length] = '\0'; // Null-terminate the word

    e->cursor = start; // Move cursor to the start of the word
    printf("Extracted word: '%s'\n", word);  // Debug print
    return true;
}

void activate_snippet(Editor *e) {
    char word[MAX_SNIPPET_KEY_LENGTH];
    size_t original_cursor_position = e->cursor; // Save the original cursor position

    if (!get_word_left_of_cursor(e, word, sizeof(word))) {
        return; // No valid word found, so do nothing.
    }

    bool snippet_found = false; // Flag to check if a snippet is found

    for (size_t i = 0; i < snippets.used; i++) {
        if (strcmp(snippets.array[i].key, word) == 0) {
            snippet_found = true; // A matching snippet is found.
            size_t word_length = strlen(word);

            // Delete the word from the buffer
            memmove(&e->data.items[e->cursor],
                    &e->data.items[e->cursor + word_length],
                    e->data.count - (e->cursor + word_length));
            e->data.count -= word_length;

            // Duplicate snippet content to manipulate
            char *snippet_content = strdup(snippets.array[i].content);
            char *placeholder_pos = strstr(snippet_content, "$0");

            // Capture the current indentation level
            size_t cursor_row = editor_row_from_pos(e, e->cursor);
            size_t line_start = e->lines.items[cursor_row].begin;
            size_t current_indent = e->cursor - line_start;

            // Calculate the position of $0
            size_t placeholder_line = 0;
            size_t placeholder_col = 0;
            if (placeholder_pos) {
                for (char *p = snippet_content; p < placeholder_pos; ++p) {
                    if (*p == '\n') {
                        placeholder_line++;
                        placeholder_col = 0;
                    } else {
                        placeholder_col++;
                    }
                }
                memmove(placeholder_pos, placeholder_pos + 2, strlen(placeholder_pos + 2) + 1);  // Remove $0
            }

            // Insert the snippet content line by line with indentation
            char *line = strtok(snippet_content, "\n");
            while (line != NULL) {
                if (cursor_row != editor_row_from_pos(e, e->cursor)) {
                    // Apply indentation for new lines
                    for (size_t i = 0; i < current_indent; ++i) {
                        editor_insert_char(e, ' ');
                    }
                }

                editor_insert_buf(e, line, strlen(line));
                line = strtok(NULL, "\n");
                if (line) {
                    editor_insert_char(e, '\n');
                }
            }

            // Adjust cursor position to where $0 was
            if (placeholder_pos) {
                e->cursor = e->lines.items[cursor_row + placeholder_line].begin + placeholder_col + (placeholder_line > 0 ? current_indent : 0);
            }

            free(snippet_content);
            break; // Exit the loop as the snippet is found and processed.
        }
    }

    if (!snippet_found) {
        e->cursor = original_cursor_position; // Restore cursor to its original position.
    }
}
