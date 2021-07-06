cmake_minimum_required (VERSION 3.8)
project (ded)

# MSVC build system configuration.
include(deps/msvc.cmake)

# TODO : Add the src folder as subfolder.
add_executable (ded
    src/editor.c src/editor.h
    src/file.c src/file.h
    src/font.c src/font.h
    src/gl_extra.c src/gl_extra.h
    src/la.c src/la.h
    src/main.c
    src/sdl_extra.c src/sdl_extra.h
    src/stb_image.h
    src/sv.h)

#
# Sets the include and linker directories for the ded project.
# TODO : Create Imported libraries for the different dependencies.
#
# set INCLUDES=/I dependencies\SDL2\include /I dependencies\GLFW\include /I dependencies\GLEW\include
# set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
#         dependencies\SDL2\lib\x64\SDL2main.lib ^
#         dependencies\GLFW\lib\glfw3.lib ^
#         dependencies\GLEW\lib\glew32s.lib ^
#         opengl32.lib User32.lib Gdi32.lib Shell32.lib

# TODO : This should not be enabled at default.
# TODO : NODEFAULTLIB
target_compile_definitions(ded 
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NODEFAULTLIB
)

target_include_directories (ded 
    PRIVATE
        ${LibSDL_INCLUDE_DIR} ${LibGLFW_INCLUDE_DIR} ${LibGLEW_INCLUDE_DIR}
)

target_link_libraries (ded 
    PRIVATE
        SDL2.lib SDL2main.lib glfw3.lib glew32s.lib
    PUBLIC
        opengl32.lib User32.lib Gdi32.lib Shell32.lib
)

