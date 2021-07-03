@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4996 /FC /TC /Zi /nologo
set INCLUDES=/I SDL2\include
set LIBS=SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib Shell32.lib

cl.exe %CFLAGS% %INCLUDES% /Fete src\main.c src\la.c src\editor.c src\font.c src\sdl_extra.c /link %LIBS% -SUBSYSTEM:windows
