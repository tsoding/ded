@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4996 /wd5105 /FC /TC /Zi /nologo
set INCLUDES=/I src /I dependencies\SDL2\include /I dependencies\freetype\include /I dependencies\GLEW\include /I dependencies\GLFW\include
set LIBS=dependencies\SDL2\lib\SDL2.lib ^
         dependencies\SDL2\lib\SDL2main.lib ^
         dependencies\freetype\lib\freetype.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         opengl32.lib User32.lib Gdi32.lib Shell32.lib

cl.exe %CFLAGS% %INCLUDES% /Feded src\main.c src\la.c src\lexer.c src\editor.c src\file_browser.c src\free_glyph.c src\simple_renderer.c src\common.c /link %LIBS% -SUBSYSTEM:windows

copy dependencies\SDL2\lib\SDL2.dll SDL2.dll
copy dependencies\freetype\lib\freetype.dll freetype.dll
