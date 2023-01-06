#!/bin/sh

set -xe

PKGS="--static sdl2 glew freetype2"
CFLAGS="-Wall -Wextra -pedantic -ggdb -DGLEW_STATIC `pkg-config --cflags $PKGS`"
LIBS="-lm -lopengl32 `pkg-config --libs $PKGS`"
SRC="src/main.c src/la.c src/editor.c src/sdl_extra.c src/file.c src/gl_extra.c src/free_glyph.c src/uniforms.c src/simple_renderer.c"
OBJ=$(echo "$SRC" | sed "s/\.c/\.o/g")
OBJ=$(echo "$OBJ" | sed "s/src\// /g")

gcc -std=c11 $CFLAGS -c $SRC
# some libs linked with c++ stuff
g++ -o life.exe $OBJ $LIBS $LIBS -static

