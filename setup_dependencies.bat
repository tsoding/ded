@echo off
@REM Don't disable command extensions

if exist dependencies rmdir /s /q dependencies
mkdir dependencies

curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
tar -xf SDL2-devel-2.0.12-VC.zip
del SDL2-devel-2.0.12-VC.zip
mkdir dependencies\SDL2
mkdir dependencies\SDL2\include
move SDL2-2.0.12\include dependencies\SDL2\include\SDL2
mkdir dependencies\SDL2\lib
move SDL2-2.0.12\lib\x64\SDL2.dll dependencies\SDL2\lib\SDL2.dll
move SDL2-2.0.12\lib\x64\SDL2.lib dependencies\SDL2\lib\SDL2.lib
move SDL2-2.0.12\lib\x64\SDL2main.lib dependencies\SDL2\lib\SDL2main.lib
rmdir /s /q SDL2-2.0.12

curl -fsSL -o glfw-3.3.2.bin.WIN64.zip https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip
tar -xf glfw-3.3.2.bin.WIN64.zip
del glfw-3.3.2.bin.WIN64.zip
mkdir dependencies\GLFW
mkdir dependencies\GLFW\lib
move glfw-3.3.2.bin.WIN64\lib-vc2019\glfw3.lib dependencies\GLFW\lib\glfw3.lib
mkdir dependencies\GLFW\include\GLFW
move glfw-3.3.2.bin.WIN64\include\GLFW\glfw3.h dependencies\GLFW\include\GLFW\glfw3.h
rmdir /s /q glfw-3.3.2.bin.WIN64

curl -fsSL -o freetype-2.13.3.tar.gz https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v2.13.3.zip
tar -xf freetype-2.13.3.tar.gz
del freetype-2.13.3.tar.gz
mkdir dependencies\freetype
move freetype-windows-binaries-2.13.3\include dependencies\freetype\include
move "freetype-windows-binaries-2.13.3\release dll\win64" dependencies\freetype\lib
rmdir /s /q freetype-windows-binaries-2.13.3

curl -fsSL -o glew-2.1.0-win32.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download
tar -xf glew-2.1.0-win32.zip
del glew-2.1.0-win32.zip
mkdir dependencies\GLEW
mkdir dependencies\GLEW\include\GL
move glew-2.1.0\include\GL\glew.h dependencies\GLEW\include\GL\glew.h
mkdir dependencies\GLEW\lib
move glew-2.1.0\lib\Release\x64\glew32s.lib dependencies\GLEW\lib\glew32s.lib
rmdir /s /q glew-2.1.0
