cmake_minimum_required(VERSION 3.24)
project(ded C)
set(CMAKE_C_STANDARD 11)

find_package(OpenGL REQUIRED)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Freetype REQUIRED)
SET(CMAKE_EXE_LINKER_FLAGS "-v")

find_package(GLEW REQUIRED)


set( SDL_STATIC ON CACHE BOOL "" FORCE )
set( SDL_SHARED OFF CACHE BOOL "" FORCE )
add_subdirectory( ${LIB_DIR}/SDL2 )



add_subdirectory(
        ${LIB_DIR}/glfw
        ${LIB_DIR}/freetype
)



set(SRC
    src/arena.h
        src/common.c
        src/common.h
        src/main.c
        src/editor.c
        src/editor.h
        src/file_browser.c
        src/file_browser.h
        src/free_glyph.c
        src/free_glyph.h
        src/la.c
        src/la.h
        src/lexer.c
        src/lexer.h
        src/minirent.h
        src/simple_renderer.c
        src/simple_renderer.h
        src/sv.h
)

add_executable(ded ${SRC})

include_directories(
        ${OPENGL_INCLUDE_DIR}
        ${FT2_DIR}
)

target_link_libraries(ded
        glfw
        OpenGL::GL
)



target_link_libraries(ded ${FREETYPE_LIBRARIES})
target_include_directories(ded PRIVATE ${FREETYPE_INCLUDE_DIRS})


target_link_libraries(ded GLEW::GLEW)

target_link_libraries( ${CMAKE_PROJECT_NAME} SDL2main SDL2-static )
