#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#define TABS_INSTEAD_OF_SPACES
#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define DRAG_X_MIN 1
#define DRAG_Y_MIN 1

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./editor.h"
#include "./file_browser.h"
#include "./la.h"
#include "./free_glyph.h"
#include "./simple_renderer.h"
#include "./common.h"
#include "./lexer.h"
#include "./sv.h"

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

// TODO: display errors reported via flash_error right in the text editor window somehow
#define flash_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)


int main(int argc, char **argv)
{
    Errno err;

    FT_Library library = {0};

    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "ERROR: Could not initialize FreeType2 library\n");
        return 1;
    }

    // TODO: users should be able to customize the font
    // const char *const font_file_path = "./fonts/VictorMono-Regular.ttf";
    const char *const font_file_path = "./fonts/iosevka-regular.ttf";

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
            fprintf(stderr, "HELP: If you're trying to open a directory, don't give any arguments, press F3 inside the editor.\n");
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

    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "ERROR: Could not initialize GLEW!");
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
            case SDL_QUIT: {
                quit = true;
            }
            break;

            case SDL_MOUSEBUTTONUP:
				if(!file_browser && (event.button.button & SDL_BUTTON_LEFT) == SDL_BUTTON_LEFT) {
					editor.drag_mouse = false;
				}
				
            	break;

            case SDL_MOUSEMOTION:
            	if(!file_browser && editor.drag_mouse) {
            		int x_rel = event.motion.xrel;
            		int y_rel = event.motion.yrel;
            		int is_x_left = x_rel < 0;
            		int is_y_down = y_rel < 0;
            		x_rel = abs(x_rel);
            		y_rel = abs(y_rel);
            		while(y_rel > DRAG_Y_MIN) {
						if(is_y_down) {
							editor_move_line_up(&editor);
						} else {
							editor_move_line_down(&editor);
						}
						
            			y_rel -= DRAG_Y_MIN;
            		}

            		while(x_rel > DRAG_X_MIN) {
            			if(event.key.keysym.mod & KMOD_CTRL) {
            				if(is_x_left) {
            					editor_move_word_left(&editor);
            				} else {
            					editor_move_word_right(&editor);
            				}
            			} else {
            				if(is_x_left) {
            					editor_move_char_left(&editor);
            				} else {
            					editor_move_char_right(&editor);
            				}
            			}

            			x_rel -= DRAG_X_MIN;
            		}

            		editor.last_stroke = SDL_GetTicks();
            	}
            	
            	break;

            case SDL_MOUSEWHEEL:
                if(event.wheel.y > 0) {
                    editor_move_line_up(&editor);
                } else if(event.wheel.y < 0) {
                    editor_move_line_down(&editor);
                }
                
                if(file_browser) {
                    if(event.wheel.y > 0) {
                        if(fb.cursor > 0) fb.cursor -= 1;
                    } else if(event.wheel.y < 0) {
                        if(fb.cursor + 1 < fb.files.count) fb.cursor += 1;
                    }
                }
                
                break;

            
            case SDL_MOUSEBUTTONDOWN:
                if(file_browser && (event.button.button & SDL_BUTTON_LEFT) == SDL_BUTTON_LEFT) {
	                const char *file_path = fb_file_path(&fb);
	                File_Type ft;
	                err = type_of_file(file_path, &ft);
	                if (err != 0) {
	                    flash_error("Could not determine type of file %s: %s", file_path, strerror(err));
	                } else {
	                    switch (ft) {
	                    case FT_DIRECTORY: {
	                        err = fb_change_dir(&fb);
	                        if (err != 0) {
	                            flash_error("Could not change directory to %s: %s", file_path, strerror(err));
	                        }
	                    }
	                    break;

	                    case FT_REGULAR: {
	                        // TODO: before opening a new file make sure you don't have unsaved changes
	                        // And if you do, annoy the user about it. (just like all the other editors do)
	                        err = editor_load_from_file(&editor, file_path);
	                        if (err != 0) {
	                            flash_error("Could not open file %s: %s", file_path, strerror(err));
	                        } else {
	                            file_browser = false;
	                        }
	                    }
	                    break;

	                    case FT_OTHER: {
	                        flash_error("%s is neither a regular file nor a directory. We can't open it.", file_path);
	                    }
	                    break;

	                    default:
	                        UNREACHABLE("unknown File_Type");
	                    }
	                }
                } else if((event.button.button & SDL_BUTTON_LEFT) == SDL_BUTTON_LEFT) {
                	editor.drag_mouse = true;
                }
                
                break;

            case SDL_KEYDOWN: {
                if (file_browser) {
                    switch (event.key.keysym.sym) {
					case SDLK_q:
						if(event.key.keysym.mod & KMOD_CTRL) {
							quit = true;
						}

						break;

                    case SDLK_ESCAPE: {
                        file_browser = false;
                    }
                    break;

					case SDLK_w:
                    case SDLK_UP: {
                        if (fb.cursor > 0) fb.cursor -= 1;
                    }
                    break;

					case SDLK_s:
                    case SDLK_DOWN: {
                        if (fb.cursor + 1 < fb.files.count) fb.cursor += 1;
                    }
                    break;

                    case SDLK_RETURN: {
                        const char *file_path = fb_file_path(&fb);
                        if (file_path) {
                            File_Type ft;
                            err = type_of_file(file_path, &ft);
                            if (err != 0) {
                                flash_error("Could not determine type of file %s: %s", file_path, strerror(err));
                            } else {
                                switch (ft) {
                                case FT_DIRECTORY: {
                                    err = fb_change_dir(&fb);
                                    if (err != 0) {
                                        flash_error("Could not change directory to %s: %s", file_path, strerror(err));
                                    }
                                }
                                break;

                                case FT_REGULAR: {
                                    // TODO: before opening a new file make sure you don't have unsaved changes
                                    // And if you do, annoy the user about it. (just like all the other editors do)
                                    err = editor_load_from_file(&editor, file_path);
                                    if (err != 0) {
                                        flash_error("Could not open file %s: %s", file_path, strerror(err));
                                    } else {
                                        file_browser = false;
                                    }
                                }
                                break;

                                case FT_OTHER: {
                                    flash_error("%s is neither a regular file nor a directory. We can't open it.", file_path);
                                }
                                break;

                                default:
                                    UNREACHABLE("unknown File_Type");
                                }
                            }
                        }
                    }
                    break;
                    }
                } else {
                    switch (event.key.keysym.sym) {
                    case SDLK_HOME: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_to_begin(&editor);
                        } else {
                            editor_move_to_line_begin(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    } break;

                    case SDLK_END: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_to_end(&editor);
                        } else {
                            editor_move_to_line_end(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    } break;

                    case SDLK_BACKSPACE: {
                        editor_backspace(&editor);
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;

					case SDLK_q:
						if(event.key.keysym.mod & KMOD_CTRL) {
							quit = true;
						}

						break;

					case SDLK_r:
						if(event.key.keysym.mod & KMOD_CTRL) {
							simple_renderer_reload_shaders(&sr);
						}

						break;
					
                    case SDLK_s: {
						if(event.key.keysym.mod & KMOD_CTRL) {
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

                    case SDLK_ESCAPE: {
                    	if(editor.searching) {
                    		editor_stop_search(&editor);
     		                editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                    	} else {
							file_browser = true;
                    	}
                    }
                    break;

                    case SDLK_RETURN: {
                        if (editor.searching) {
                            editor_stop_search(&editor);
                        } else {
                            editor_insert_char(&editor, '\n');
                            editor.last_stroke = SDL_GetTicks();
                        }
                    }
                    break;

                    case SDLK_DELETE: {
                        editor_delete(&editor);
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;

                    case SDLK_f: {
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_start_search(&editor);
                        }
                    }
                    break;

                    case SDLK_a: {
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor.selection = true;
                            editor.select_begin = 0;
                            editor.cursor = editor.data.count;
                        }
                    }
                    break;

                    case SDLK_TAB: {
                        // XXX: Tabs are kind of a hack, needs a redo.
                        #ifdef TABS_INSTEAD_OF_SPACES
                            editor_insert_char(&editor, '\t');
                        #else
                            for(int i = 0; i < TAB_SIZE; i++) {
                                editor_insert_char(&editor, ' ');
                            }
                        #endif
                    }
                    break;

                    case SDLK_c: {
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_clipboard_copy(&editor);
                        }
                    }
                    break;

                    case SDLK_v: {
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_clipboard_paste(&editor);
                        }
                    }
                    break;

                    case SDLK_UP: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_paragraph_up(&editor);
                        } else {
                            editor_move_line_up(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;

                    case SDLK_DOWN: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_paragraph_down(&editor);
                        } else {
                            editor_move_line_down(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;

                    case SDLK_LEFT: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_word_left(&editor);
                        } else {
                            editor_move_char_left(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;

                    case SDLK_RIGHT: {
                        editor_update_selection(&editor, event.key.keysym.mod & KMOD_SHIFT);
                        if (event.key.keysym.mod & KMOD_CTRL) {
                            editor_move_word_right(&editor);
                        } else {
                            editor_move_char_right(&editor);
                        }
                        editor.last_stroke = SDL_GetTicks();
                    }
                    break;
                    }
                }
            }
            break;

            case SDL_TEXTINPUT: {
                if (file_browser) {
                    // Nothing for now
                    // Once we have incremental search in the file browser this may become useful
                } else {
                    const char *text = event.text.text;
                    size_t text_len = strlen(text);
                    for (size_t i = 0; i < text_len; ++i) {
                        editor_insert_char(&editor, text[i]);
                    }
                    editor.last_stroke = SDL_GetTicks();
                }
            }
            break;
            }
        }

        

        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            // TODO(#19): update the viewport and the resolution only on actual window change
            glViewport(0, 0, w, h);
        }

        Vec4f bg = hex_to_vec4f(0x181818FF);
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
