#include <assert.h>
#include "./uniforms.h"

static_assert(COUNT_UNIFORM_SLOTS == 7, "The amount of the shader uniforms have change. Please update the definition table accordingly");
static const Uniform_Def uniform_defs[COUNT_UNIFORM_SLOTS] = {
    [UNIFORM_SLOT_TIME] = {
        .slot = UNIFORM_SLOT_TIME,
        .name = "time",
    },
    [UNIFORM_SLOT_RESOLUTION] = {
        .slot = UNIFORM_SLOT_RESOLUTION,
        .name = "resolution",
    },
    [UNIFORM_SLOT_CAMERA_POS] = {
        .slot = UNIFORM_SLOT_CAMERA_POS,
        .name = "camera_pos",
    },
    [UNIFORM_SLOT_CAMERA_SCALE] = {
        .slot = UNIFORM_SLOT_CAMERA_SCALE,
        .name = "camera_scale",
    },
    [UNIFORM_SLOT_CURSOR_POS] = {
        .slot = UNIFORM_SLOT_CURSOR_POS,
        .name = "cursor_pos",
    },
    [UNIFORM_SLOT_CURSOR_HEIGHT] = {
        .slot = UNIFORM_SLOT_CURSOR_HEIGHT,
        .name = "cursor_height",
    },
    [UNIFORM_SLOT_LAST_STROKE] = {
        .slot = UNIFORM_SLOT_LAST_STROKE,
        .name = "last_stroke",
    },
};

const Uniform_Def *get_uniform_def(Uniform_Slot slot)
{
    assert(0 <= slot);
    assert(slot < COUNT_UNIFORM_SLOTS);
    return &uniform_defs[slot];
}

void get_uniform_location(GLuint program, GLint locations[COUNT_UNIFORM_SLOTS])
{
    for (Uniform_Slot slot = 0; slot < COUNT_UNIFORM_SLOTS; ++slot) {
        locations[slot] = glGetUniformLocation(program, uniform_defs[slot].name);
    }
}
