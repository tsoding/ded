#include <string.h>
#include "file_browser.h"
#include "simple_renderer.h"
#include "sv.h"
#include "editor.h" // only for zoom_factor maybe im bad at programming
#include <stdbool.h>
#include <sys/stat.h>


bool file_browser = false;


static int file_cmp(const void *ap, const void *bp)
{
    const char *a = *(const char**)ap;
    const char *b = *(const char**)bp;
    return strcmp(a, b);
}

Errno fb_open_dir(File_Browser *fb, const char *dir_path)
{
    char resolved_path[PATH_MAX];
    expand_path(dir_path, resolved_path, sizeof(resolved_path));

    fb->files.count = 0;
    fb->cursor = 0;
    Errno err = read_entire_dir(resolved_path, &fb->files);
    if (err != 0) {
        return err;
    }
    qsort(fb->files.items, fb->files.count, sizeof(*fb->files.items), file_cmp);

    fb->dir_path.count = 0;
    sb_append_cstr(&fb->dir_path, resolved_path);
    sb_append_null(&fb->dir_path);
    printf("Opened directory: %s\n", fb->dir_path.items);
    return 0;
}


#define PATH_SEP "/"
#define PATH_EMPTY ""
#define PATH_DOT "."
#define PATH_DOTDOT ".."

typedef struct {
    String_View *items;
    size_t count;
    size_t capacity;
} Comps;

void normpath(String_View path, String_Builder *result)
{
    size_t original_sb_size = result->count;

    if (path.count == 0) {
        sb_append_cstr(result, PATH_DOT);
        return;
    }

    int initial_slashes = 0;
    while (path.count > 0 && *path.data == *PATH_SEP) {
        initial_slashes += 1;
        sv_chop_left(&path, 1);
    }
    if (initial_slashes > 2) {
        initial_slashes = 1;
    }

    Comps new_comps = {0};

    while (path.count > 0) {
        String_View comp = sv_chop_by_delim(&path, '/');
        if (comp.count == 0 || sv_eq(comp, SV(PATH_DOT))) {
            continue;
        }
        if (!sv_eq(comp, SV(PATH_DOTDOT))) {
            da_append(&new_comps, comp);
            continue;
        }
        if (initial_slashes == 0 && new_comps.count == 0) {
            da_append(&new_comps, comp);
            continue;
        }
        if (new_comps.count > 0 && sv_eq(da_last(&new_comps), SV(PATH_DOTDOT))) {
            da_append(&new_comps, comp);
            continue;
        }
        if (new_comps.count > 0) {
            new_comps.count -= 1;
            continue;
        }
    }

    for (int i = 0; i < initial_slashes; ++i) {
        sb_append_cstr(result, PATH_SEP);
    }

    for (size_t i = 0; i < new_comps.count; ++i) {
        if (i > 0) sb_append_cstr(result, PATH_SEP);
        sb_append_buf(result, new_comps.items[i].data, new_comps.items[i].count);
    }

    if (original_sb_size == result->count) {
        sb_append_cstr(result, PATH_DOT);
    }

    free(new_comps.items);
}

Errno fb_change_dir(File_Browser *fb)
{
    assert(fb->dir_path.count > 0 && "You need to call fb_open_dir() before fb_change_dir()");
    assert(fb->dir_path.items[fb->dir_path.count - 1] == '\0');

    if (fb->cursor >= fb->files.count) return 0;

    const char *dir_name = fb->files.items[fb->cursor];
    char new_path[PATH_MAX];
    snprintf(new_path, sizeof(new_path), "%s/%s", fb->dir_path.items, dir_name);

    char resolved_path[PATH_MAX];
    expand_path(new_path, resolved_path, sizeof(resolved_path));

    fb->dir_path.count = 0;
    sb_append_cstr(&fb->dir_path, resolved_path);
    sb_append_null(&fb->dir_path);

    fb->files.count = 0;
    fb->cursor = 0;
    Errno err = read_entire_dir(resolved_path, &fb->files);
    if (err != 0) {
        return err;
    }
    qsort(fb->files.items, fb->files.count, sizeof(*fb->files.items), file_cmp);
    printf("Changed directory to: %s\n", fb->dir_path.items);
    return 0;
}

void fb_render(const File_Browser *fb, SDL_Window *window, Free_Glyph_Atlas *atlas, Simple_Renderer *sr)
{
    Vec2f cursor_pos = vec2f(0, -(float)fb->cursor * FREE_GLYPH_FONT_SIZE);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float max_line_len = 0.0f;

    sr->resolution = vec2f(w, h);
    sr->time = (float) SDL_GetTicks() / 1000.0f;

    simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_COLOR);
    if (fb->cursor < fb->files.count) {
        const Vec2f begin = vec2f(0, -((float)fb->cursor + CURSOR_OFFSET) * FREE_GLYPH_FONT_SIZE);
        Vec2f end = begin;
        free_glyph_atlas_measure_line_sized(
            atlas, fb->files.items[fb->cursor], strlen(fb->files.items[fb->cursor]),
            &end);
        simple_renderer_solid_rect(sr, begin, vec2f(end.x - begin.x, FREE_GLYPH_FONT_SIZE), vec4f(.25, .25, .25, 1));


    }
    simple_renderer_flush(sr);

    simple_renderer_set_shader(sr, VERTEX_SHADER_SIMPLE, SHADER_FOR_EPICNESS);
    for (size_t row = 0; row < fb->files.count; ++row) {
        const Vec2f begin = vec2f(0, -(float)row * FREE_GLYPH_FONT_SIZE);
        Vec2f end = begin;
        free_glyph_atlas_render_line_sized(
            atlas, sr, fb->files.items[row], strlen(fb->files.items[row]),
            &end,
            vec4fs(0));
        // TODO: the max_line_len should be calculated based on what's visible on the screen right now
        float line_len = fabsf(end.x - begin.x);
        if (line_len > max_line_len) {
            max_line_len = line_len;
        }
    }

    simple_renderer_flush(sr);

    // Update camera
    {

        if (followCursor) {

            if (max_line_len > 1000.0f) {
                max_line_len = 1000.0f;
            }

            float target_scale = w/ zoom_factor /(max_line_len*0.75); // TODO: division by 0

            Vec2f target = cursor_pos;
            float offset = 0.0f;

            if (target_scale > 3.0f) {
                target_scale = 3.0f;
            } else {
                offset = cursor_pos.x - w/1.0f/sr->camera_scale;
                if (offset < 0.0f) offset = 0.0f;
                target = vec2f(w/3.0f/sr->camera_scale + offset, cursor_pos.y);
            }

            sr->camera_vel = vec2f_mul(
                                       vec2f_sub(target, sr->camera_pos),
                                       vec2fs(2.0f));
            sr->camera_scale_vel = (target_scale - sr->camera_scale) * 2.0f;

            sr->camera_pos = vec2f_add(sr->camera_pos, vec2f_mul(sr->camera_vel, vec2fs(DELTA_TIME)));
            sr->camera_scale = sr->camera_scale + sr->camera_scale_vel * DELTA_TIME;
        }
        // TODO // else if !followCursor
    }
}

const char *fb_file_path(File_Browser *fb)
{
    assert(fb->dir_path.count > 0 && "You need to call fb_open_dir() before fb_file_path()");
    assert(fb->dir_path.items[fb->dir_path.count - 1] == '\0');

    if (fb->cursor >= fb->files.count) return NULL;

    fb->file_path.count = 0;
    sb_append_buf(&fb->file_path, fb->dir_path.items, fb->dir_path.count - 1);
    sb_append_buf(&fb->file_path, "/", 1);
    sb_append_cstr(&fb->file_path, fb->files.items[fb->cursor]);
    sb_append_null(&fb->file_path);

    extract_file_extension(fb->files.items[fb->cursor], &fb->file_extension);
    printf("File path: %s\n", fb->file_path.items); // Print file path
    printf("File extension: %s\n", fb->file_extension.items); // Print file extension

    return fb->file_path.items;
}

// ADDED
void extract_file_extension(const char *filename, String_Builder *ext) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        // No extension found or the dot is the first character (hidden files in Unix)
        // Clear the String_Builder manually
        ext->count = 0;
        sb_append_null(ext);
        return;
    }
    // Clear the String_Builder manually before appending new content
    ext->count = 0;
    sb_append_cstr(ext, dot + 1); // Skip the dot
    sb_append_null(ext); // Ensure null termination
}

void expand_path(const char *original_path, char *expanded_path, size_t expanded_path_size) {
    if (original_path[0] == '~') {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(expanded_path, expanded_path_size, "%s%s", home, original_path + 1);
        } else {
            strncpy(expanded_path, original_path, expanded_path_size);
        }
    } else {
        char resolved_path[PATH_MAX];
        if (realpath(original_path, resolved_path) != NULL) {
            strncpy(expanded_path, resolved_path, expanded_path_size);
        } else {
            strncpy(expanded_path, original_path, expanded_path_size);
        }
    }
    expanded_path[expanded_path_size - 1] = '\0';
}

/* static int is_directory(const char* base_path, const char* file) { */
/*     char full_path[PATH_MAX]; */
/*     snprintf(full_path, PATH_MAX, "%s/%s", base_path, file); */

/*     struct stat statbuf; */
/*     if (stat(full_path, &statbuf) != 0) { */
/*         return 0;  // In case of error, assume it's not a directory */
/*     } */

/*     return S_ISDIR(statbuf.st_mode); */
/* } */

void toggle_file_browser(){
    file_browser = !file_browser;
}    

