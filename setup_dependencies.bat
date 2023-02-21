@echo off

curl -fsSL -o https://github.com/libsdl-org/SDL/releases/download/release-2.26.3/SDL2-2.26.3.zip
tar -xf SDL2-2.26.3.zip
if not exist dependencies\ mkdir dependencies\
move SDL2-2.0.12 dependencies\SDL2
del SDL2-devel-2.0.12-VC.zip
if not exist dependencies\SDL2\temp\ mkdir dependencies\SDL2\temp\
move dependencies\SDL2\include dependencies\SDL2\temp\SDL2
move dependencies\SDL2\temp dependencies\SDL2\include


git clone https://github.com/glfw/glfw dependencies/GLFW

curl -fsSL -o glew-2.1.0-win32.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download
tar -xf glew-2.1.0.zip
if not exist dependencies\GLEW\lib\ mkdir dependencies\GLEW\lib\
move glew-2.1.0\lib\Release\x64\glew32s.lib dependencies\GLEW\lib\glew32s.lib
if not exist dependencies\GLEW\include\GL\ mkdir dependencies\GLEW\include\GL\
move glew-2.1.0\include\GL\glew.h dependencies\GLEW\include\GL\glew.h
del glew-2.1.0-win32.zip
rmdir /s /q glew-2.1.0

git clone https://github.com/ubawurinna/freetype-windows-binaries.git dependencies/freetype