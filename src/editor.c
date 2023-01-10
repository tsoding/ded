#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "./editor.h"
#include "./sv.h"
#include "common.h"

void editor_backspace(Editor *e)
{
    if (e->cursor > e->data.count) {
        e->cursor = e->data.count;
    }
    if (e->cursor == 0) return;

    memmove(
        &e->data.items[e->cursor - 1],
        &e->data.items[e->cursor],
        e->data.count - e->cursor
    );
    e->cursor -= 1;
    e->data.count -= 1;
    editor_recompute_lines(e);
}

void editor_delete(Editor *e)
{
    if (e->cursor >= e->data.count) return;
    memmove(
        &e->data.items[e->cursor],
        &e->data.items[e->cursor + 1],
        e->data.count - e->cursor - 1
    );
    e->data.count -= 1;
    editor_recompute_lines(e);
}

Errno editor_save_as(Editor *editor, const char *file_path)
{
    Errno err = write_entire_file(file_path, editor->data.items, editor->data.count);
    if (err != 0) return err;
    editor->file_path.count = 0;
    sb_append_cstr(&editor->file_path, file_path);
    sb_append_null(&editor->file_path);
    return 0;
}

Errno editor_save(const Editor *editor)
{
    assert(editor->file_path.count > 0);
    return write_entire_file(editor->file_path.items, editor->data.items, editor->data.count);
}

static Errno file_size(FILE *file, size_t *size)
{
    long saved = ftell(file);
    if (saved < 0) return errno;
    if (fseek(file, 0, SEEK_END) < 0) return errno;
    long result = ftell(file);
    if (result < 0) return errno;
    if (fseek(file, saved, SEEK_SET) < 0) return errno;
    *size = (size_t) result;
    return 0;
}

Errno editor_load_from_file(Editor *e, const char *file_path)
{
    Errno result = 0;
    FILE *file = NULL;

    e->data.count = 0;

    file = fopen(file_path, "rb");
    if (file == NULL) return_defer(errno);

    size_t data_size;
    Errno err = file_size(file, &data_size);
    if (err != 0) return_defer(err);

    if (e->data.capacity < data_size) {
        e->data.capacity = data_size;
        e->data.items = realloc(e->data.items, e->data.capacity*sizeof(*e->data.items));
        assert(e->data.items != NULL && "Buy more RAM lol");
    }

    fread(e->data.items, data_size, 1, file);
    if (ferror(file)) return_defer(errno);
    e->data.count = data_size;

    editor_recompute_lines(e);

defer:
    if (result == 0) {
        e->file_path.count = 0;
        sb_append_cstr(&e->file_path, file_path);
        sb_append_null(&e->file_path);
    }
    if (file) fclose(file);
    return result;
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
    if (e->cursor > 0) e->cursor -= 1;
}

void editor_move_char_right(Editor *e)
{
    if (e->cursor < e->data.count) e->cursor += 1;
}

void editor_insert_char(Editor *e, char x)
{
    if (e->cursor > e->data.count) {
        e->cursor = e->data.count;
    }

    da_append(&e->data, '\0');
    memmove(
        &e->data.items[e->cursor + 1],
        &e->data.items[e->cursor],
        e->data.count - e->cursor - 1
    );
    e->data.items[e->cursor] = x;
    e->cursor += 1;

    editor_recompute_lines(e);
}

void editor_recompute_lines(Editor *e)
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
