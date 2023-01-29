@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4996 /wd5105 /wd4244 /wd4267 /wd4702 /wd4200 /wd4090 /FC /TC /Zi /nologo
set LDFLAGS=/ignore:4099 /subsystem:windows
set INCLUDES=/I dependencies\SDL2\include /I dependencies\GLFW\include /I dependencies\GLEW\include /I dependencies\freetype\include
set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
         dependencies\SDL2\lib\x64\SDL2main.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         dependencies\freetype\lib\freetype.lib ^
         opengl32.lib User32.lib Gdi32.lib Shell32.lib

if not exist SDL2.dll copy dependencies\SDL2\lib\x64\SDL2.dll SDL2.dll || exit /b

cl.exe %CFLAGS% %INCLUDES% /Feded src\common.c src\editor.c src\file_browser.c src\free_glyph.c src\la.c src\lexer.c src\main.c src\simple_renderer.c /link %LIBS% %LDFLAGS%
