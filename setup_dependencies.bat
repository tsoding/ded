@echo off
cd /D "%~dp0"

echo Setting up SDL2...
curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
tar -xf SDL2-devel-2.0.12-VC.zip
if not exist dependencies\SDL2\include\SDL2 mkdir dependencies\SDL2\include\SDL2
move /y SDL2-2.0.12\include\* dependencies\SDL2\include\SDL2\ > nul
move /y SDL2-2.0.12\lib dependencies\SDL2\lib > nul
del SDL2-devel-2.0.12-VC.zip
rmdir /s /q SDL2-2.0.12

echo Setting up GLFW...
curl -fsSL -o glfw-3.3.2.bin.WIN64.zip https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip
tar -xf glfw-3.3.2.bin.WIN64.zip
if not exist dependencies\GLFW\lib\ mkdir dependencies\GLFW\lib\
move /y glfw-3.3.2.bin.WIN64\lib-vc2019\glfw3.lib dependencies\GLFW\lib\glfw3.lib > nul
if not exist dependencies\GLFW\include\GLFW mkdir dependencies\GLFW\include\GLFW
move /y glfw-3.3.2.bin.WIN64\include\GLFW\glfw3.h dependencies\GLFW\include\GLFW\glfw3.h > nul
del glfw-3.3.2.bin.WIN64.zip
rmdir /s /q glfw-3.3.2.bin.WIN64

echo Setting up GLEW...
curl -fsSL -o glew-2.1.0-win32.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download
tar -xf glew-2.1.0-win32.zip
if not exist dependencies\GLEW\lib\ mkdir dependencies\GLEW\lib\
move /y glew-2.1.0\lib\Release\x64\glew32s.lib dependencies\GLEW\lib\glew32s.lib > nul
if not exist dependencies\GLEW\include\GL\ mkdir dependencies\GLEW\include\GL\
move /y glew-2.1.0\include\GL\glew.h dependencies\GLEW\include\GL\glew.h > nul
del glew-2.1.0-win32.zip
rmdir /s /q glew-2.1.0

echo Setting up freetype...
curl -fsSL -o freetype-windows-binaries-2.12.1.zip https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v2.12.1.zip
tar -xf freetype-windows-binaries-2.12.1.zip
if not exist dependencies\freetype mkdir dependencies\freetype
move /y freetype-windows-binaries-2.12.1\include dependencies\freetype\include > nul
if not exist dependencies\freetype\lib mkdir dependencies\freetype\lib
move /y "freetype-windows-binaries-2.12.1\release static\vs2015-2022\win64\freetype.lib" dependencies\freetype\lib\freetype.lib > nul
del freetype-windows-binaries-2.12.1.zip
rmdir /s /q freetype-windows-binaries-2.12.1
