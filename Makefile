CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm
SRCS=src/main.c src/la.c src/editor.c src/font.c src/sdl_extra.c

te: $(SRCS)
	$(CC) $(CFLAGS) -o te $(SRCS) $(LIBS)
