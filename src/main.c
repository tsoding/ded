#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include "common.h"

#include <dirent.h>

#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./editor.h"
#include "./repl.h"
#include "./file_browser.h"
#include "./la.h"
#include "./free_glyph.h"
#include "./simple_renderer.h"
#include "./common.h"
#include "./lexer.h"
#include "./sv.h"

// added
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define FONT_DIR "~/.config/ded/fonts/"
/* #define DEFAULT_FONT "jet-extra-bold.ttf" */
#define DEFAULT_FONT "radon.otf"
/* #define DEFAULT_FONT "Letters.ttf" */
/* #define DEFAULT_FONT "designer.otf" */
#define MAX_FONTS 20
#define MAX_PATH_SIZE 1024

char *fonts[MAX_FONTS];
int font_count = 0;
int current_font_index = 0;


// TODO: Save file dialog
// Needed when ded is ran without any file so it does not know where to save.

// TODO: An ability to create a new file
// TODO: Delete a word
// TODO: Delete selection
// TODO: Undo/redo system

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam)
{
    (void) source;
    (void) id;
    (void) length;
    (void) userParam;
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

static Free_Glyph_Atlas atlas = {0};
static Simple_Renderer sr = {0};
static Editor editor = {0};
static File_Browser fb = {0};


FT_Face load_font_face(FT_Library library, const char *font_name, FT_UInt pixel_size) {
    printf("Loading font: %s at index: %d\n", font_name, current_font_index);
    char font_path[MAX_PATH_SIZE];
    const char *homeDir = getenv("HOME");
    snprintf(font_path, sizeof(font_path), "%s/.config/ded/fonts/%s", homeDir, font_name);

    FT_Face face;
    FT_Error error = FT_New_Face(library, font_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_path);
        exit(1);
    } else if (error) {
        fprintf(stderr, "ERROR: Could not load file `%s`\n", font_path);
        exit(1);
    }

    error = FT_Set_Pixel_Sizes(face, 0, pixel_size); // Set pixel size for the loaded font face
    if (error) {
        fprintf(stderr, "ERROR: Could not set pixel size to %u\n", pixel_size);
        return NULL; // or handle the error in a different way
    }

    return face;
}

void prev_font() {
    if (current_font_index == 0) {
        // Already at the first font, don't do anything.
        return;
    }
    current_font_index--;
}

void next_font() {
    if (current_font_index == font_count - 1) {
        // Already at the last font, don't do anything.
        return;
    }
    current_font_index++;
}


void populate_font_list() {
    char path[MAX_PATH_SIZE];
    const char *homeDir = getenv("HOME");
    if (!homeDir) {
        fprintf(stderr, "ERROR: Could not get HOME directory\n");
        exit(1);
    }

    snprintf(path, sizeof(path), "%s/.config/ded/fonts/", homeDir);

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ERROR: Could not open directory `%s`\n", path);
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) && font_count < MAX_FONTS) {
        if (entry->d_type == DT_REG) {  // If the entry is a regular file
            fonts[font_count] = strdup(entry->d_name);
            font_count++;
        }
    }
    closedir(dir);
}



void switch_to_font(FT_Library library, FT_Face *currentFace, Free_Glyph_Atlas *atlas, int direction) {
    if (direction > 0) {
        next_font();
    } else {
        prev_font();
    }
    /* *currentFace = load_font_face(library, fonts[current_font_index]); */
    *currentFace = load_font_face(library, fonts[current_font_index], FREE_GLYPH_FONT_SIZE);


    // Dispose the old texture
    /* glDeleteTextures(1, &atlas->glyphs_texture); */

    // Reinitialize the atlas with the new font face
    free_glyph_atlas_init(atlas, *currentFace);
}



// TODO tomove
bool extractLine(Editor *editor, size_t cursor, char *line, size_t max_length) {
    size_t start = cursor;
    while (start > 0 && editor->data.items[start - 1] != '\n') {
        start--;
    }

    size_t end = start;
    while (end < editor->data.count && editor->data.items[end] != '\n') {
        end++;
    }

    size_t length = end - start;
    if (length < max_length) {
        strncpy(line, &editor->data.items[start], length);
        line[length] = '\0';
        return true;
    }

    return false;
}

bool extractLocalIncludePath(Editor *editor, char *includePath) {
    char line[512]; // Adjust size as needed
    if (!extractLine(editor, editor->cursor, line, sizeof(line))) {
        return false;
    }

    if (strncmp(line, "#include \"", 10) == 0) {
        char *start = strchr(line, '\"') + 1;
        char *end = strrchr(line, '\"');
        if (start && end && start < end) {
            size_t length = end - start;
            strncpy(includePath, start, length);
            includePath[length] = '\0';
            return true;
        }
    }

    return false;
}

void getDirectoryFromFilePath(const char *filePath, char *directory) {
    strcpy(directory, filePath);
    char *lastSlash = strrchr(directory, '/');
    if (lastSlash != NULL) {
        *lastSlash = '\0'; // Null-terminate at the last slash
    } else {
        // Handle the case where there is no slash in the path
        strcpy(directory, ".");
    }
}

// Function to open a local include file
Errno openLocalIncludeFile(Editor *editor, const char *includePath) {
    char fullPath[512]; // Buffer for the full path
    char directory[256]; // Buffer for the directory

    // Get the directory of the current file
    getDirectoryFromFilePath(editor->file_path.items, directory);

    // Construct the full path
    snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, includePath);

    // Load the file using the full path
    Errno load_err = editor_load_from_file(editor, fullPath);
    if (load_err != 0) {
        fprintf(stderr, "Error loading file %s: %s\n", fullPath, strerror(load_err));
        return load_err;
    }

    printf("Opened file: %s\n", fullPath);
    return 0;
}










// TODO: display errors reported via flash_error right in the text editor window somehow
#define flash_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)


int main(int argc, char **argv)
{

  initialize_themes();
  initialize_shader_paths();
    Errno err;

    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: Could not initialize FreeType2 library\n");
        return 1;
    }

    // TODO: users should be able to customize the font
    /* const char *const font_file_path = "./fonts/VictorMono-Regular.ttf"; */
    /* const char *const font_file_path = "./fonts/jet-bold.ttf"; */
    /* const char *const font_file_path = "~/.config/ded/fonts/jet-extra-bold.ttf"; */
    /* const char *const font_file_path = "./fonts/iosevka-regular.ttf"; */


    /* char font_file_path_buffer[1024]; */
    /* const char *homeDir = getenv("HOME"); */
    /* if (homeDir) { */
    /*   snprintf(font_file_path_buffer, sizeof(font_file_path_buffer), "%s/.config/ded/fonts/minecraft_font.ttf", homeDir); */
    /* } else { */
    /*   // handle the error, for now, we'll just set it to the original value as a fallback */
    /*   strncpy(font_file_path_buffer, "~/.config/ded/fonts/jet-extra-bold.ttf", sizeof(font_file_path_buffer)); */
    /* } */
    /* const char *const font_file_path = font_file_path_buffer; */



    populate_font_list();

    if (font_count == 0) {
      fprintf(stderr, "ERROR: No fonts found in `%s`\n", FONT_DIR);
      return 1;
    }

    // Start with the default font
    for (int i = 0; i < font_count; i++) {
      if (strcmp(fonts[i], DEFAULT_FONT) == 0) {
        current_font_index = i;
        break;
      }
    }

    /* FT_Face face = load_font_face(library, fonts[current_font_index]); */
    FT_Face face = load_font_face(library, fonts[current_font_index], FREE_GLYPH_FONT_SIZE);







    /* original */
    /* FT_Face face; */
    /* error = FT_New_Face(library, font_file_path, 0, &face); */
    /* if (error == FT_Err_Unknown_File_Format) { */
    /*     fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_file_path); */
    /*     return 1; */
    /* } else if (error) { */
    /*     fprintf(stderr, "ERROR: Could not load file `%s`\n", font_file_path); */
    /*     return 1; */
    /* } */


    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "ERROR: Could not set pixel size to %u\n", pixel_size);
        return 1;
    }

    if (argc > 1) {
        const char *file_path = argv[1];
        err = editor_load_from_file(&editor, file_path);
        if (err != 0) {
            fprintf(stderr, "ERROR: Could not read file %s: %s\n", file_path, strerror(err));
            return 1;
        }
    }

    const char *dir_path = ".";
    err = fb_open_dir(&fb, dir_path);
    if (err != 0) {
        fprintf(stderr, "ERROR: Could not read directory %s: %s\n", dir_path, strerror(err));
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("ded",
                         0, 0,
                         SCREEN_WIDTH, SCREEN_HEIGHT,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "ERROR: Could not create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        int major;
        int minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        printf("GL version %d.%d\n", major, minor);
    }

    if (SDL_GL_CreateContext(window) == NULL) {
        fprintf(stderr, "ERROR: Could not create OpenGL context: %s\n", SDL_GetError());
        return 1;
    }

    GLenum glewErr = glewInit();
    if (GLEW_OK != glewErr) {
        fprintf(stderr, "ERROR: Could not initialize GLEW: %s\n", glewGetErrorString(glewErr));
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } else {
        fprintf(stderr, "WARNING: GLEW_ARB_debug_output is not available");
    }

    simple_renderer_init(&sr);
    free_glyph_atlas_init(&atlas, face);

    editor.atlas = &atlas;
    editor_retokenize(&editor);





    bool quit = false;
    bool file_browser = false;

    while (!quit) {
        const Uint32 start = SDL_GetTicks();
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
              quit = true;
              break;

            case SDL_KEYDOWN:
              if (file_browser) {
                switch (event.key.keysym.sym) {
                case SDLK_F3: {
                  file_browser = false;
                } break;

                case SDLK_EQUALS: {
                  if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                    theme_next(&currentThemeIndex);
                    printf("Changed theme to %d\n", currentThemeIndex); // Logging the theme change for debugging
                  } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                    zoom_factor -= 0.8f;

                    if (zoom_factor < min_zoom_factor) {
                      zoom_factor = min_zoom_factor;
                    }
                  }
                } break;

                case SDLK_MINUS: {
                  if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                    theme_previous(&currentThemeIndex);
                    printf("Changed theme back to %d\n", currentThemeIndex); // Logging the theme change for debugging
                  } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                    zoom_factor += 0.8f;

                    if (zoom_factor > max_zoom_factor) {
                      zoom_factor = max_zoom_factor;
                    }
                  }
                } break;

                case SDLK_q:
                case SDLK_ESCAPE: {
                  file_browser = false;
                } break;


                case SDLK_r:
                  if (event.key.keysym.mod & KMOD_CTRL) {
                    file_browser = false;
                  }
                  break;


                case SDLK_F5: {
                  simple_renderer_reload_shaders(&sr);
                }
                  break;

                case SDLK_UP: {
                  if (fb.cursor > 0)
                    fb.cursor -= 1;
                } break;

                case SDLK_k: {
                  if (fb.cursor > 0)
                    fb.cursor -= 1;
                } break;

                case SDLK_DOWN: {
                  if (fb.cursor + 1 < fb.files.count)
                    fb.cursor += 1;
                } break;

                case SDLK_j: {
                  if (fb.cursor + 1 < fb.files.count)
                    fb.cursor += 1;
                } break;

                case SDLK_h: {
                  // Copy current directory path
                  char current_dir[PATH_MAX];
                  strncpy(current_dir, fb.dir_path.items, fb.dir_path.count);
                  current_dir[fb.dir_path.count - 1] = '\0'; // Ensure null-termination

                  // Get parent directory
                  char *parent = dirname(current_dir);

                  // Open parent directory
                  Errno err = fb_open_dir(&fb, parent);
                  if (err != 0) {
                    // Handle error, for example, print out an error message.
                  } else {
                    fb.cursor = 0; // Reset cursor position in the new directory
                  }
                } break;

                case SDLK_t: {
                  if (SDL_GetModState() & KMOD_CTRL) {
                    isAnimated = !isAnimated;  // Toggle the state
                  }
                }
                  break;


                case SDLK_l: {
                  const char *file_path = fb_file_path(&fb);
                  if (file_path) {
                    File_Type ft;
                    err = type_of_file(file_path, &ft);
                    if (err != 0) {
                      flash_error("Could not determine type of file %s: %s",
                                  file_path, strerror(err));
                    } else {
                      switch (ft) {
                      case FT_DIRECTORY: {
                        err = fb_change_dir(&fb);
                        if (err != 0) {
                          flash_error("Could not change directory to %s: %s",
                                      file_path, strerror(err));
                        }
                      } break;

                      case FT_REGULAR: {
                        // TODO: before opening a new file make sure you don't
                        // have unsaved changes And if you do, annoy the user
                        // about it. (just like all the other editors do)
                        err = editor_load_from_file(&editor, file_path);
                        if (err != 0) {
                          flash_error("Could not open file %s: %s", file_path,
                                      strerror(err));
                        } else {
                          file_browser = false;
                        }
                      } break;

                      case FT_OTHER: {
                        flash_error("%s is neither a regular file nor a "
                                    "directory. We can't open it.",
                                    file_path);
                      } break;

                      default:
                        UNREACHABLE("unknown File_Type");
                      }
                    }
                  }
                }
                break;
                }
              } else {
                switch (current_mode) {
                case NORMAL:
                  switch (event.key.keysym.sym) {
                    SDL_Event tmpEvent; // Declare once at the beginning of the switch block

                  /* case SDLK_RETURN: { */
                  /*   if (editor.searching) { */
                  /*     editor_stop_search_and_mark(&editor); */
                  /*     current_mode = NORMAL; */
                  /*   } else { */
                  /*     // buffer to hold the extracted word. */
                  /*     // Assuming the maximum word length is 255 characters. */
                  /*     char word[256]; */

                  /*     // If the word is successfully extracted, print it to the */
                  /*     // debug output. */
                  /*     if (extractWordUnderCursor(&editor, word)) { */
                  /*       printf("Extracted word: %s\n", word); */
                  /*     } else { */
                  /*       printf("No word under cursor\n"); */
                  /*     } */
                  /*   } */
                  /* } break; */

                  case SDLK_RETURN: {
                    if (editor.searching) {
                      editor_stop_search_and_mark(&editor);
                      current_mode = NORMAL;
                    } else {
                      char includePath[256];
                      if (extractLocalIncludePath(&editor, includePath)) {
                        openLocalIncludeFile(&editor, includePath);
                      }
                    }
                  } break;

                  case SDLK_ESCAPE: {
                    editor_clear_mark(&editor);
                    editor_stop_search(&editor);
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                  }
                    break;

                  case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) { // Scroll up
                      printf("Scroll Up event captured\n"); // Debug print
                      move_camera(&sr, "up", 20.0f); // Notice &sr, passed the address of sr
                    } else if (event.wheel.y < 0) { // Scroll down
                      printf("Scroll Down event captured\n"); // Debug print
                      move_camera(&sr, "down", 20.0f); // Notice &sr, passed the address of sr
                    }
                    break;





                  case SDLK_LEFTBRACKET:
                    if (SDL_GetModState() & KMOD_ALT) {
                      switch_to_font(library, &face, &atlas, -1);
                      printf("Switched to previous font: %s\n", fonts[current_font_index]);
                      /* redraw_screen(); */
                    }
                    break;

                  case SDLK_RIGHTBRACKET:
                    if (SDL_GetModState() & KMOD_ALT) {
                      switch_to_font(library, &face, &atlas, 1);
                      printf("Switched to next font: %s\n", fonts[current_font_index]);
                      /* redraw_screen(); */
                    }
                    break;


                    case SDLK_TAB: {
                        // TODO: indent on Tab instead of just inserting 4 spaces at the cursor
                        // That is insert the spaces at the beginning of the line. Shift+TAB should
                        // do unindent, that is remove 4 spaces from the beginning of the line.
                        // TODO: customizable indentation style
                        // - tabs/spaces
                        // - tab width
                        // - etc.
                        for (size_t i = 0; i < 4; ++i) {
                            editor_insert_char(&editor, ' ');
                        }
                    }
                    break;

                  case SDLK_z: {
                    if (SDL_GetModState() & KMOD_CTRL) {
                      showLineNumbers = !showLineNumbers;  // Toggle the state of showLineNumbers
                    }
                  }
                    break;

                  case SDLK_t: {
                    if (SDL_GetModState() & KMOD_CTRL) {
                      isAnimated = !isAnimated;  // Toggle the state
                    }
                  }
                    break;


                    case SDLK_F5: {
                        simple_renderer_reload_shaders(&sr);
                    }
                    break;

                  case SDLK_y:
                    editor_clipboard_copy(&editor);
                    break;

                  case SDLK_g: {
                    if (SDL_GetModState() & KMOD_SHIFT) {
                      editor_move_to_end(&editor);
                    } else {
                      editor_move_to_begin(&editor);
                    }
                  } break;

                  case SDLK_SLASH: {
                    current_mode = INSERT;
                    editor_start_search(&editor);

                    // Consume the next SDL_TEXTINPUT event for '/'
                    SDL_Event tmpEvent;
                    SDL_PollEvent(&tmpEvent);
                    if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '/') {
                      SDL_PushEvent(&tmpEvent); // Push the event back if it's not the one we're trying to consume
                    }
                  } break;

                  case SDLK_n: {
                    if (SDL_GetModState() & KMOD_SHIFT) {
                      editor_search_previous(&editor);
                    } else if(editor.has_mark){
                      editor_search_next(&editor);
                    }
                    if (SDL_GetModState() & KMOD_CTRL) {
                      editor_move_line_down(&editor);
                    }
                  } break;

                  case SDLK_p:
                    if (SDL_GetModState() & KMOD_CTRL){
                      editor_move_line_up(&editor);
                    } else {
                      editor_clipboard_paste(&editor);
                    }
                    break;

                  case SDLK_b:
                    if (SDL_GetModState() & KMOD_CTRL){
                      editor_move_char_left(&editor);
                    }
                    break;

                  case SDLK_f:
                    if (SDL_GetModState() & KMOD_CTRL){
                      editor_move_char_right(&editor);
                    }
                    break;

                  case SDLK_EQUALS: {
                    if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                      theme_next(&currentThemeIndex);
                      printf("Changed theme to %d\n", currentThemeIndex); // Logging the theme change for debugging
                    } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                      zoom_factor -= 1.0f;
                      if (zoom_factor < min_zoom_factor) {
                        zoom_factor = min_zoom_factor;
                      }
                    }
                  } break;

                  case SDLK_MINUS: {
                    if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                      theme_previous(&currentThemeIndex);
                      printf("Changed theme back to %d\n", currentThemeIndex); // Logging the theme change for debugging
                    } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                      zoom_factor += 1.0f;
                      if (zoom_factor > max_zoom_factor) {
                        zoom_factor = max_zoom_factor;
                      }
                    }
                  } break;

                     // TODO mouse support
                  /* case SDL_MOUSEWHEEL: */
                  /*   if (event.wheel.y > 0 && SDL_GetModState() & KMOD_CTRL) {  // Mouse wheel scrolled up with Ctrl held */
                  /*     zoom_factor -= 0.1f;  // Adjust the zoom factor for zooming in */
                  /*     if (zoom_factor < 1.0f) zoom_factor = 1.0f;  // Ensure zoom_factor doesn't drop below a threshold */
                  /*   } else if (event.wheel.y < 0 && SDL_GetModState() & KMOD_CTRL) {  // Mouse wheel scrolled down with Ctrl held */
                  /*     zoom_factor += 0.1f;  // Adjust the zoom factor for zooming out */
                  /*   } */
                  /*   break; */

                    case SDLK_i:
                      current_mode = INSERT;

                      // Reset the blink timer
                      editor.last_stroke = SDL_GetTicks();
                      /* is_animated = true;  // TODO make this an option and smooth */

                      // Eat up the next SDL_TEXTINPUT event for 'i'
                      SDL_PollEvent(&tmpEvent); // This will typically be the SDL_TEXTINPUT event for 'i'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != 'i') {
                        SDL_PushEvent(&tmpEvent); // If it's not, push it back to the event queue
                      }
                      break;

                  case SDLK_v: {
                    if (SDL_GetModState() & KMOD_SHIFT) {
                      current_mode = VISUAL_LINE;
                      editor_start_visual_line_selection(&editor);  // Initiate line selection.
                    } else {
                      current_mode = VISUAL;
                      editor_start_visual_selection(&editor);  // Initiate character selection.
                    }
                  } break;


                  case SDLK_a:
                    editor.last_stroke = SDL_GetTicks();
                    if (SDL_GetModState() & KMOD_SHIFT) { // Check if shift is being held
                      editor_move_to_line_end(&editor);
                    } else {
                      // Move the cursor one position to the right
                      editor_move_char_right(&editor);
                    }

                    // Enter INSERT mode
                    current_mode = INSERT;

                    // Eat up the next SDL_TEXTINPUT event for 'a' or 'A'
                    SDL_PollEvent(&tmpEvent); // This will typically be the SDL_TEXTINPUT event for 'a' or 'A'
                    if (tmpEvent.type != SDL_TEXTINPUT || (tmpEvent.text.text[0] != 'a' && tmpEvent.text.text[0] != 'A')) {
                      SDL_PushEvent(&tmpEvent); // If it's not, push it back to the event queue
                    }
                    break;

                      // Enter INSERT mode
                      current_mode = INSERT;

                      // Eat up the next SDL_TEXTINPUT event for 'a'
                      SDL_PollEvent(&tmpEvent); // This will typically be the SDL_TEXTINPUT event for 'a'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != 'a') {
                        SDL_PushEvent(&tmpEvent); // If it's not, push it back to the event queue
                      }
                      break;

                  case SDLK_x:
                    if (editor.selection) {
                      editor_clipboard_copy(&editor);
                      editor_delete_selection(&editor);
                      editor.selection = false;
                    } else {
                      editor_clipboard_copy(&editor);
                      editor_cut_char_under_cursor(&editor);
                    }
                    break;

                  case SDLK_0:
                    editor_move_to_line_begin(&editor);
                    break;

                  case SDLK_F3:
                    file_browser = true;
                    break;

                  case SDLK_r:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                    file_browser = true;
                    }
                    break;

                  case SDLK_BACKSPACE:       //  yes you can delete in normal mode
                    editor_backspace(&editor);
                    break;

                  case SDLK_h:  // Left
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_left(&editor);
                    } else {
                      editor_move_char_left(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_j:  // Down
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if ((event.key.keysym.mod & KMOD_ALT) && !isAnimated) {
                      move_camera(&sr, "down", 50.0f);
                    } else if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_down(&editor);
                    } else {
                      editor_move_line_down(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_k:
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if ((event.key.keysym.mod & KMOD_ALT) && !isAnimated) {
                      move_camera(&sr, "up", 50.0f);
                    } else if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_up(&editor);
                    } else {
                      editor_move_line_up(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_l:
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_right(&editor);
                    } else {
                      editor_move_char_right(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_w:
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    editor_move_word_right(&editor);
                    break;

                    // Add additional NORMAL mode keybinds here...
                  } break;

                case INSERT:
                  switch (event.key.keysym.sym) {

                  case SDLK_h:  // Left
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_char_left(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_j:  // Down
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_line_down(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_k:  // Up
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_line_up(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_l:  // Right
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_char_right(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                    case SDLK_TAB: {
                        // TODO: indent on Tab instead of just inserting 4 spaces at the cursor
                        // That is insert the spaces at the beginning of the line. Shift+TAB should
                        // do unindent, that is remove 4 spaces from the beginning of the line.
                        // TODO: customizable indentation style
                        // - tabs/spaces
                        // - tab width
                        // - etc.
                        for (size_t i = 0; i < 4; ++i) {
                            editor_insert_char(&editor, ' ');
                        }
                    }
                    break;

                  case SDLK_F3:
                    file_browser = true;
                    break;

                  case SDLK_MINUS:
                    if (SDL_GetModState() & KMOD_CTRL) {
                      zoom_factor += 1.0f;


                      if (zoom_factor > max_zoom_factor) {
                        zoom_factor = max_zoom_factor;
                      }

                      printf("zoom_factor = %f\n", zoom_factor);
                      // Consume the next SDL_TEXTINPUT event for '-'
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);
                      if (!(tmpEvent.type == SDL_TEXTINPUT && tmpEvent.text.text[0] == '-')) {
                        SDL_PushEvent(&tmpEvent);  // Push the event back if it's not the one we're trying to consume
                      }
                    }
                    break;

                  case SDLK_EQUALS:
                    if (SDL_GetModState() & KMOD_CTRL) {
                      zoom_factor -= 1.0f;

                      printf("zoom_factor = %f\n", zoom_factor);

                      if (zoom_factor < min_zoom_factor) {
                        zoom_factor = min_zoom_factor;
                      }

                      // Consume the next SDL_TEXTINPUT event for '='
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);
                      if (!(tmpEvent.type == SDL_TEXTINPUT && tmpEvent.text.text[0] == '=')) {
                        SDL_PushEvent(&tmpEvent);  // Push the event back if it's not the one we're trying to consume
                      }
                    }
                    break;

                  case SDLK_9: {
                    if (event.key.keysym.mod & KMOD_SHIFT) {
                      char pair[] = "()";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor);

                      // Consume both characters '(' and ')' immediately
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);  // Consume '('
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '(') {
                        SDL_PushEvent(&tmpEvent);
                      }
                      SDL_PollEvent(&tmpEvent);  // Consume ')'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != ')') {
                        SDL_PushEvent(&tmpEvent);
                      }
                    } else {
                      editor_insert_char(&editor, '9');

                      // Consume the next SDL_TEXTINPUT event for '9'
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '9') {
                        SDL_PushEvent(&tmpEvent);
                      }
                    }
                  }
                    break;

                  case SDLK_LEFTBRACKET: {
                    if (event.key.keysym.mod & KMOD_SHIFT) {
                      char pair[] = "{}";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor);

                      // Consume both characters '{' and '}' immediately
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);  // Consume '{'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '{') {
                        SDL_PushEvent(&tmpEvent);
                      }
                      SDL_PollEvent(&tmpEvent);  // Consume '}'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '}') {
                        SDL_PushEvent(&tmpEvent);
                      }
                    } else {
                      // Insert two '[' characters, move the cursor left, and consume the keypress
                      char pair[] = "[]";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor);
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent); // Consume '['
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '[') {
                        SDL_PushEvent(&tmpEvent); // Push the event back if it's not the one we're trying to consume
                      }
                    }
                  }
                    break;

                  case SDLK_QUOTE: {
                    if (event.key.keysym.mod & KMOD_SHIFT) {
                      // If Shift + ' is pressed, insert double quotes ""
                      char pair[] = "\"\"";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor);

                      // Consume both characters '"' and '"' immediately
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent);  // Consume first '"'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '\"') {
                        SDL_PushEvent(&tmpEvent);
                      }
                      SDL_PollEvent(&tmpEvent);  // Consume second '"'
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '\"') {
                        SDL_PushEvent(&tmpEvent);
                      }
                    } else {
                      // If just ' is pressed, insert single quotes ''
                      char pair[] = "''";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor);
                      SDL_Event tmpEvent;
                      SDL_PollEvent(&tmpEvent); // Consume first '''
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '\'') {
                        SDL_PushEvent(&tmpEvent); // Push the event back if it's not the one we're trying to consume
                      }
                    }
                  }
                    break;

                  case SDLK_BACKSPACE:
                    editor_backspace(&editor);
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_RETURN: {
                      if (editor.searching) {
                           editor_stop_search_and_mark(&editor);
                           current_mode = NORMAL;

                       } else {
                           editor_insert_char(&editor, '\n');
                           editor.last_stroke = SDL_GetTicks();
                       }
                   }
                   break;




                    case SDLK_f: {
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_start_search(&editor);
                        }
                    }
                    break;

                    case SDLK_ESCAPE: {
                        current_mode = NORMAL;
                        editor_clear_mark(&editor);
                        editor_stop_search(&editor);
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    }
                    break;

                  case SDLK_c:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_clipboard_copy(&editor);
                    }
                    break;

                  case SDLK_s: {
                    if (SDL_GetModState() & KMOD_CTRL) {  // Checks if CTRL is held down
                      if (editor.file_path.count > 0) {
                        err = editor_save(&editor);
                        if (err != 0) {
                          flash_error("Could not save currently edited file: %s", strerror(err));
                        }
                      } else {
                        // TODO: ask the user for the path to save to in this situation
                        flash_error("Nowhere to save the text");
                      }
                    }
                  }
                    break;

                    case SDLK_F5: {
                        simple_renderer_reload_shaders(&sr);
                    }
                    break;

                  case SDLK_v:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_clipboard_paste(&editor);
                    }
                    break;

                  case SDLK_UP:
                    editor_update_selection(&editor,
                                            event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_up(&editor);
                    } else {
                      editor_move_line_up(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_DOWN:
                    editor_update_selection(&editor,
                                            event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_down(&editor);
                    } else {
                      editor_move_line_down(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_LEFT:
                    editor_update_selection(&editor,
                                            event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_left(&editor);
                    } else {
                      editor_move_char_left(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_RIGHT:
                    editor_update_selection(&editor,
                                            event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_right(&editor);
                    } else {
                      editor_move_char_right(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;
                  }
                  break;


                case VISUAL:
                  switch (event.key.keysym.sym) {

                  /* case SDLK_x: */
                  /*     editor_delete_selection(&editor); */
                  /*     editor.selection = false; */
                  /*   break; */


                  case SDLK_x:
                    if (editor.selection) {
                      editor_clipboard_copy(&editor);
                      editor_delete_selection(&editor);
                      editor.selection = false;
                      current_mode = NORMAL;

                    }
                    break;


                  case SDLK_j:  // Down
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_down(&editor);
                    } else {
                      editor_move_line_down(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;


                  case SDLK_h:  // Left
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_left(&editor);
                    } else {
                      editor_move_char_left(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_k:  // Up
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_up(&editor);
                    } else {
                      editor_move_line_up(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  case SDLK_l:  // Right
                    editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_word_right(&editor);
                    } else {
                      editor_move_char_right(&editor);
                    }
                    editor.last_stroke = SDL_GetTicks();
                    break;

                  //  transition back to NORMAL mode
                  case SDLK_ESCAPE:
                    editor.selection = false;
                    current_mode = NORMAL;
                    break;
                  }
                  break;

                    // Add additional VISUAL mode keybinds here...

            case VISUAL_LINE:
              switch (event.key.keysym.sym) {

              case SDLK_j:  // Down
                editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                if (event.key.keysym.mod & KMOD_CTRL) {
                  editor_move_paragraph_down(&editor);
                } else {
                  editor_move_line_down(&editor);
                }
                editor.last_stroke = SDL_GetTicks();
                break;

              case SDLK_h:  // Left
                editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                if (event.key.keysym.mod & KMOD_CTRL) {
                  editor_move_word_left(&editor);
                } else {
                  editor_move_char_left(&editor);
                }
                editor.last_stroke = SDL_GetTicks();
                break;

              case SDLK_k:  // Up
                editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                if (event.key.keysym.mod & KMOD_CTRL) {
                  editor_move_paragraph_up(&editor);
                } else {
                  editor_move_line_up(&editor);
                }
                editor.last_stroke = SDL_GetTicks();
                break;

              case SDLK_l:  // Right
                editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                if (event.key.keysym.mod & KMOD_CTRL) {
                  editor_move_word_right(&editor);
                } else {
                  editor_move_char_right(&editor);
                }
                editor.last_stroke = SDL_GetTicks();
                break;

                // Transition back to NORMAL mode
              case SDLK_ESCAPE:
                current_mode = NORMAL;
                break;

                // Add additional VISUAL_LINE mode keybinds here...
              }
              break;

              // More cases for other modes can follow here...
              // ...
                }
                break;
              }
              break;


            case SDL_TEXTINPUT:
              if (file_browser) {
                // Once we have incremental search in the file browser this may become useful
              } else if (current_mode == INSERT) { // Process text input only in INSERT mode
                const char *text = event.text.text;
                size_t text_len = strlen(text);
                for (size_t i = 0; i < text_len; ++i) {
                  editor_insert_char(&editor, text[i]);
                }
                editor.last_stroke = SDL_GetTicks();
              }
              break;

            }
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        Vec4f bg = themes[currentThemeIndex].background;

        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT);

        if (file_browser) {
          fb_render(&fb, window, &atlas, &sr);
        } else {
          editor_render(window, &atlas, &sr, &editor);
        }

        SDL_GL_SwapWindow(window);
        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 delta_time_ms = 1000 / FPS;
        if (duration < delta_time_ms) {
            SDL_Delay(delta_time_ms - duration);
        }
    }
    return 0;
}

// TODO: ability to search within file browser
// Very useful when you have a lot of files
// TODO: ability to remove trailing whitespaces
