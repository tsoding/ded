@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /std:c11 /wd4996 /wd5105 /FC /TC /Zi /nologo
set INCLUDES=/I dependencies\SDL2\include /I dependencies\GLFW\include /I dependencies\GLEW\include
set BASE_FILES=src\main.c src\la.c src\editor.c src\font.c src\sdl_extra.c src\file.c src\gl_extra.c
set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
         dependencies\SDL2\lib\x64\SDL2main.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         opengl32.lib User32.lib Gdi32.lib Shell32.lib

if not exist build\ mkdir build\
if not exist build\SDL2.dll copy dependencies\SDL2\lib\x64\SDL2.dll build\SDL2.dll

cl.exe %CFLAGS% %INCLUDES% %BASE_FILES% /Fobuild\ /Febuild\ded /link %LIBS% -SUBSYSTEM:windows
