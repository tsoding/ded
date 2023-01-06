# Dramatic EDitor

**THIS SOFTWARE IS UNFINISHED!!!**

# Quick Start

## POSIX

```console
$ ./build.sh
$ ./ded src\main.c
```

## Windows MSVC

```console
> .\setup_dependencies.bat
> .\build_msvc.bat
> .\ded.exe src\main.c
```

## msys2 in windows
you can use msys2 and mingw64 for build this app into native windows exe. 
Run `mingw64.exe` after msys2 installation.
Add packaged into it: `pacman -S git base-devel mingw-w64-x86_64-gcc mingw-w64-x86_64-glew mingw-w64-x86_64-mesa mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkgconf mingw-w64-x86_64-freetype`
Change dir by `cd "path_copypasted_from_explorer_without_final_slash"`
Run script `./build_msys2_mingw64.sh`. The resulting `life.exe` is 6 MB in size.


# Font

Victor Mono: https://rubjo.github.io/victor-mono/
