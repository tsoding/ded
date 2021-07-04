#!/bin/sh

set -xe

CC="${CXX:-cc}"
PKGS="sdl2 glew"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb"
LIBS=-lm
SRC="src/main.c src/la.c src/editor.c src/font.c src/sdl_extra.c src/file.c src/gl_extra.c"

if [ `uname` = "Darwin" ]; then
    CFLAGS+=" -framework OpenGL"
fi

$CC $CFLAGS `pkg-config --cflags $PKGS` -o te $SRC $LIBS `pkg-config --libs $PKGS`
