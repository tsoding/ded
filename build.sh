#!/bin/sh

set -xe

CC="${CXX:-cc}"
PKGS="sdl2 freetype2"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb -Ideps"
LIBS=-lm
SRC="src/main.c src/la.c src/editor.c src/file_browser.c src/free_glyph.c src/simple_renderer.c src/common.c src/lexer.c"

if [ `uname` = "Darwin" ]; then
    CFLAGS+=" -framework OpenGL"
else
    PKGS="$PKGS gl"
fi


$CC $CFLAGS `pkg-config --cflags $PKGS` -o ded $SRC $LIBS `pkg-config --libs $PKGS`
