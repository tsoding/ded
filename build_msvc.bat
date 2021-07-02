@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /FC /TC /Zi /nologo
set INCLUDES=/I SDL2\include
set LIBS=SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib Shell32.lib

cl.exe %CFLAGS% %INCLUDES% /Fete main.c la.c /link %LIBS% -SUBSYSTEM:windows
