# Dramatic EDitor

# Quick Start 

## POSIX

```console
$ ./build.sh
$ ./ded src\main.c
```

## Windows MSYS2

```Msys2 console
> pacman -S git base-devel mingw-w64-i686-gcc mingw-w64-i686-glew mingw-w64-i686-mesa mingw-w64-i686-SDL2 mingw-w64-i686-pkg-config mingw-w64-i686-freetype
> ./build.sh --static "-static -lopengl32 -lstdc++"
> .\ded.exe src\main.c
```

# Font

Victor Mono: https://rubjo.github.io/victor-mono/

