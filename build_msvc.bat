@echo off
cd /D "%~dp0"
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4090 /wd4200 /wd4244 /wd4267 /wd4702 /wd4996 /wd5105 /FC /TC /Zi /nologo
set LDFLAGS=/ignore:4099 /subsystem:windows

set INCLUDES=/I dependencies\freetype\include ^
             /I dependencies\GLEW\include ^
             /I dependencies\GLFW\include ^
             /I dependencies\SDL2\include

set LIBS=dependencies\freetype\lib\freetype.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\SDL2\lib\x64\SDL2main.lib ^
         dependencies\SDL2\lib\x64\SDL2.lib ^
         opengl32.lib User32.lib Gdi32.lib Shell32.lib

set SRC=src\common.c ^
        src\editor.c ^
        src\file_browser.c ^
        src\free_glyph.c ^
        src\la.c ^
        src\lexer.c ^
        src\main.c ^
        src\simple_renderer.c

if not exist SDL2.dll copy dependencies\SDL2\lib\x64\SDL2.dll SDL2.dll > nul || exit /b

cl.exe %CFLAGS% %INCLUDES% /Feded %SRC% /link %LDFLAGS% %LIBS%
