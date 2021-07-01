CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm

te: main.c
	$(CC) $(CFLAGS) -o te main.c la.c $(LIBS)
