#ifndef FILE_BROWSER_H_
#define FILE_BROWSER_H_

#include "common.h"
#include "free_glyph.h"

#include <SDL2/SDL.h>

#include <stdbool.h>

typedef struct {
    Files files;
    size_t cursor;
    String_Builder dir_path;
    String_Builder file_path;
    String_Builder file_extension;

    // for file creation mode
    bool is_in_file_creation_mode;
    char tmp_filename[PATH_MAX];
    size_t tmp_filename_len;

} File_Browser;

Errno fb_open_dir(File_Browser *fb, const char *dir_path);
Errno fb_change_dir(File_Browser *fb);
void fb_render(const File_Browser *fb, SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr);

const char *fb_file_path(File_Browser *fb);

Errno fb_go_to_parent(File_Browser *fb);

// ADDED
void extract_file_extension(const char *filename, String_Builder *ext);



#endif // FILE_BROWSER_H_
