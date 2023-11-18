#!/bin/bash


if [ ! -d "$HOME/.config/ded" ]; then
    cp -r ./config/ded "$HOME/.config/"
else
    echo "Config already exists."
fi

set -xe

CC="${CXX:-cc}"
PKGS="sdl2 glew freetype2"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb -D_DEFAULT_SOURCE"
LIBS=-lm
# SRC="src/main.c src/la.c src/editor.c src/file_browser.c src/free_glyph.c src/simple_renderer.c src/common.c src/lexer.c src/keychords.c"
SRC="src/main.c src/la.c src/editor.c src/file_browser.c src/free_glyph.c src/simple_renderer.c src/common.c src/lexer.c"

if [ `uname` = "Darwin" ]; then
    CFLAGS+=" -framework OpenGL"
fi

$CC $CFLAGS `pkg-config --cflags $PKGS` -o ded $SRC $LIBS `pkg-config --libs $PKGS`



