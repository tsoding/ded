#ifndef GL_EXTRA_H_
#define GL_EXTRA_H_

#define GLEW_STATIC
#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <stdbool.h>

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader);
bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader);
bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program);

#endif // GL_EXTRA_H_
