#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include "common.h"


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
#include <stdbool.h>
#include "keychords.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))



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
static Repl repl = {0};


// TODO: display errors reported via flash_error right in the text editor window somehow
#define flash_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)

KeySequence currentSequence = {0};  // This initializes a zero-length sequence


int main(int argc, char **argv)
{

  initialize_themes();

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


    char font_file_path_buffer[1024];
    const char *homeDir = getenv("HOME");
    if (homeDir) {
      snprintf(font_file_path_buffer, sizeof(font_file_path_buffer), "%s/.config/ded/fonts/jet-extra-bold.ttf", homeDir);
    } else {
      // handle the error, for now, we'll just set it to the original value as a fallback
      strncpy(font_file_path_buffer, "~/.config/ded/fonts/jet-extra-bold.ttf", sizeof(font_file_path_buffer));
    }
    const char *const font_file_path = font_file_path_buffer;


    FT_Face face;
    error = FT_New_Face(library, font_file_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR: `%s` has an unknown format\n", font_file_path);
        return 1;
    } else if (error) {
        fprintf(stderr, "ERROR: Could not load file `%s`\n", font_file_path);
        return 1;
    }

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
    bool repl = false;
    static bool file_creation_mode = false;    // To track if we're in "file creation mode"
    static char new_filename[PATH_MAX] = "";   // To accumulate filename
    static size_t filename_cursor = 0;         // Cursor for where we're writing in new_filename

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


                case SDLK_F5: {
                  simple_renderer_reload_shaders(&sr);
                }
                  break;

                case SDLK_EQUALS: {
                  if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                    theme_next(&currentThemeIndex);
                    printf("Changed theme to %d\n", currentThemeIndex); // Logging the theme change for debugging
                  }
                } break;

                case SDLK_MINUS: {
                  if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                    theme_previous(&currentThemeIndex);
                    printf("Changed theme back to %d\n", currentThemeIndex); // Logging the theme change for debugging
                  }
                } break;

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
                  if (!file_creation_mode) {
                    file_creation_mode = true;
                    memset(new_filename, 0, sizeof(new_filename));  // Reset filename buffer
                    filename_cursor = 0;
                  }
                } break;


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

                      // WARNING
                      case SDLK_RETURN: {
                        if (file_creation_mode) {
                          // Create the new file with name in new_filename buffer in the current directory
                          char full_path[PATH_MAX];
                          snprintf(full_path, sizeof(full_path), "%s/%s", fb.dir_path.items, new_filename);

                          FILE *new_file = fopen(full_path, "w");
                          if (new_file) {
                            fclose(new_file);
                            file_creation_mode = false;  // Exit file creation mode after creating file
                            // Optional: Refresh the file browser to show the new file
                          } else {
                            flash_error("Failed to create file %s: %s", full_path, strerror(errno));
                          }
                        } else {
                          // Whatever behavior you want for RETURN key when not in file_creation_mode
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

                    case SDLK_F5: {
                        simple_renderer_reload_shaders(&sr);
                    }
                    break;

                  case SDLK_COLON: // Assuming SDLK_COLON is the correct enum value for ':', adjust if not
                    current_mode = COMMAND;

                    // Move cursor to the bottom left
                    // You would replace this with actual code to move your cursor.
                    // move_cursor_to_bottom_left();

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
                    } else {
                      editor_search_next(&editor);
                    }
                  } break;


                  case SDLK_EQUALS: {
                    if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                      theme_next(&currentThemeIndex);
                      printf("Changed theme to %d\n", currentThemeIndex); // Logging the theme change for debugging
                    } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                      zoom_factor -= 0.8f;
                    }
                  } break;

                  case SDLK_MINUS: {
                    if (SDL_GetModState() & KMOD_ALT) {  // Check if ALT is pressed
                      theme_previous(&currentThemeIndex);
                      printf("Changed theme back to %d\n", currentThemeIndex); // Logging the theme change for debugging
                    } else if (SDL_GetModState() & KMOD_CTRL) {  // Check if CTRL is pressed
                      zoom_factor += 0.8f;
                      if (zoom_factor < 1.0f) zoom_factor = 1.0f;  // Ensure zoom_factor doesn't drop below a threshold
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

                  /* case SDLK_x: */
                  /*   editor_cut_char_under_cursor(&editor); */
                  /*   break; */

                  case SDLK_x:
                    if (editor.selection) {
                      editor_delete_selection(&editor);
                      editor.selection = false;
                    } else {
                      editor_cut_char_under_cursor(&editor);
                    }
                    break;


                  case SDLK_p:
                    editor_clipboard_paste(&editor);
                    break;

                  case SDLK_0:
                    editor_move_to_line_begin(&editor);
                    break;

                  case SDLK_F1:
                    repl = true;
                    break;

                  case SDLK_F3:
                    file_browser = true;
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
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_move_paragraph_down(&editor);
                    } else {
                      editor_move_line_down(&editor);
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

                    // Add additional NORMAL mode keybinds here...
                  }
                  break;

                case INSERT:
                  switch (event.key.keysym.sym) {

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

                  case SDLK_MINUS:
                    if (SDL_GetModState() & KMOD_CTRL) {
                      zoom_factor -= 2;

                      if (zoom_factor < 1.0f) {
                        zoom_factor = 1.0f;
                      }

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
                      zoom_factor += 2;  // Increase by a factor of 0.2

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

                  case SDLK_LESS: {
                    if (event.key.keysym.mod & KMOD_SHIFT) {
                      // Handle '<' with shift pressed
                      char pair[] = "<>";
                      editor_insert_buf(&editor, pair, 2);
                      editor_move_char_left(&editor); // Move cursor between angle brackets
                    } else {
                      // Handle '<' without shift.
                      editor_insert_char(&editor, '<');
                    }

                    // Consume the upcoming character events
                    SDL_Event tmpEvent;
                    SDL_PollEvent(&tmpEvent); // Consume the first character
                    if (tmpEvent.type != SDL_TEXTINPUT || (tmpEvent.text.text[0] != '<' && tmpEvent.text.text[0] != '>')) {
                      SDL_PushEvent(&tmpEvent); // Push the event back if it's not the one we're trying to consume
                    }
                    if (event.key.keysym.mod & KMOD_SHIFT) { // Only try to consume the second character if shift was pressed
                      SDL_PollEvent(&tmpEvent); // Consume the second character
                      if (tmpEvent.type != SDL_TEXTINPUT || tmpEvent.text.text[0] != '>') {
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



                    // TODO QUOTE

                  case SDLK_BACKSPACE:
                    editor_backspace(&editor);
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
                        editor_stop_search(&editor);
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    }
                    break;


                  case SDLK_c:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                      editor_clipboard_copy(&editor);
                    }
                    break;

                  /* case SDLK_s: */
                  /*   if (event.key.keysym.mod & KMOD_CTRL) { */
                  /*     editor_save(&editor); */
                  /*   } */
                  /*   break; */

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
                    current_mode = NORMAL;
                    break;
                  }
                  break;

                    // Add additional VISUAL mode keybinds here...
                  }
                  break;

                }
              break;

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
        /* Vec4f bg = hex_to_vec4f(0x181818FF); // FDABCF   181818 */
        Vec4f bg = themes[currentThemeIndex].background;
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT);

        /* if (file_browser) { */
        /*     fb_render(&fb, window, &atlas, &sr); */
        /* } else { */
        /*     editor_render(window, &atlas, &sr, &editor); */
        /* } */


        if (file_browser) {
          fb_render(&fb, window, &atlas, &sr);
        } else if (repl) {
          repl_render(&repl, window, &atlas, &sr);  // Add the &atlas parameter here
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
