@echo off

curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
tar -xf SDL2-devel-2.0.12-VC.zip
if not exist dependencies\ mkdir dependencies\
move SDL2-2.0.12 dependencies\SDL2
del SDL2-devel-2.0.12-VC.zip
if not exist dependencies\SDL2\temp\ mkdir dependencies\SDL2\temp\
move dependencies\SDL2\include dependencies\SDL2\temp\SDL2
move dependencies\SDL2\temp dependencies\SDL2\include

curl -fsSL -o glfw-3.3.2.bin.WIN64.zip https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip
tar -xf glfw-3.3.2.bin.WIN64.zip
if not exist dependencies\GLFW\lib\ mkdir dependencies\GLFW\lib\
move glfw-3.3.2.bin.WIN64\lib-vc2019\glfw3.lib dependencies\GLFW\lib\glfw3.lib
if not exist dependencies\GLFW\include\GLFW mkdir dependencies\GLFW\include\GLFW
move glfw-3.3.2.bin.WIN64\include\GLFW\glfw3.h dependencies\GLFW\include\GLFW\glfw3.h
del glfw-3.3.2.bin.WIN64.zip
rmdir /s /q glfw-3.3.2.bin.WIN64

curl -fsSL -o glew-2.1.0-win32.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download
tar -xf glew-2.1.0-win32.zip
if not exist dependencies\GLEW\lib\ mkdir dependencies\GLEW\lib\
move glew-2.1.0\lib\Release\x64\glew32s.lib dependencies\GLEW\lib\glew32s.lib
if not exist dependencies\GLEW\include\GL\ mkdir dependencies\GLEW\include\GL\
move glew-2.1.0\include\GL\glew.h dependencies\GLEW\include\GL\glew.h
del glew-2.1.0-win32.zip
rmdir /s /q glew-2.1.0
