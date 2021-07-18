#ifndef UNIFORMS_H_
#define UNIFORMS_H_

#define GLEW_STATIC
#include <GL/glew.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

typedef enum {
    UNIFORM_SLOT_TIME = 0,
    UNIFORM_SLOT_RESOLUTION,
    UNIFORM_SLOT_CAMERA_POS,
    UNIFORM_SLOT_CAMERA_SCALE,

    UNIFORM_SLOT_CURSOR_POS,
    UNIFORM_SLOT_CURSOR_HEIGHT,
    UNIFORM_SLOT_LAST_STROKE,
    COUNT_UNIFORM_SLOTS,
} Uniform_Slot;

typedef struct {
    Uniform_Slot slot;
    const char *name;
} Uniform_Def;

const Uniform_Def *get_uniform_def(Uniform_Slot slot);
void get_uniform_location(GLuint program, GLint locations[COUNT_UNIFORM_SLOTS]);

#endif // UNIFORMS_H_
