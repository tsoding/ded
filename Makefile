PKGS=sdl2 glew
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
SRCS=src/main.c src/la.c src/editor.c src/font.c src/sdl_extra.c src/file.c src/gl_extra.c

te: $(SRCS)
	$(CC) $(CFLAGS) -o te $(SRCS) $(LIBS)
