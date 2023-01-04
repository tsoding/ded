@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4996 /wd5105 /wd4459 /wd4267 /wd4244 /FC /TC /Zi /nologo
set INCLUDES=/I dependencies\SDL2\include /I dependencies\GLFW\include /I dependencies\GLEW\include /I dependencies\freetype2\include
set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
         dependencies\SDL2\lib\x64\SDL2main.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         "dependencies\freetype2\release dll\win64\freetype.lib" ^
         opengl32.lib User32.lib Gdi32.lib Shell32.lib

cl.exe %CFLAGS% %INCLUDES% /Feded src\main.c src\la.c src\editor.c src\sdl_extra.c src\file.c src\gl_extra.c src\free_glyph.c src\simple_renderer.c src/uniforms.c /link %LIBS% -SUBSYSTEM:console
