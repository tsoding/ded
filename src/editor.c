#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "./editor.h"
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

Errno editor_save_as(Editor *e, const char *file_path)
{
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
    return write_entire_file(e->file_path.items, e->data.items, e->data.count);
}

Errno editor_load_from_file(Editor *e, const char *file_path)
{
    e->data.count = 0;
    Errno err = read_entire_file(file_path, &e->data);
    if (err != 0) return err;

    e->cursor = 0;

    editor_recompute_lines(e);

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
