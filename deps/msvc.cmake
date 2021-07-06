#
# Configures the build system for MSVC.
# Supports VS 15 and 16.
#
# Add support for x86 and x64, different glfw libs.
# Change the current file cmake configuration to
# ExternalProject_Add if the following build is used.
#

set (LibSDL_DIR "${CMAKE_SOURCE_DIR}/deps/SDL2-2.0.12")
set (LibGLFW_DIR "${CMAKE_SOURCE_DIR}/deps/glfw-3.3.2.bin.WIN64")
set (LibGLEW_DIR "${CMAKE_SOURCE_DIR}/deps/glew-2.1.0")

# Set the path variables for the libraries used as dependencies.
# SDL
set (LibSDL_LIB_DIR "${LibSDL_DIR}/lib/x86")
set (LibSDL_INCLUDE_DIR "${LibSDL_DIR}/include")

# GLFW
if (MSVC_TOOLSET_VERSION EQUAL 141)
    set (LibGLFW_LIB_DIR "${LibGLFW_DIR}/lib-vc2017")
elseif (MSVC_TOOLSET_VERSION EQUAL 142)
    set (LibGLFW_LIB_DIR "${LibGLFW_DIR}/lib-vc2019")
endif ()

set (LibGLFW_INCLUDE_DIR "${LibGLFW_DIR}/include")

# GLEW
set (LibGLEW_LIB_DIR "${LibGLEW_DIR}/lib/Release/Win32")
set (LibGLEW_INCLUDE_DIR "${LibGLEW_DIR}/include")

# Libraries for linkage.
set (LibSDL_LIBRARIES SDL2main.lib;SDL2.lib)
set (LibGLEW_LIBRARIES glfw3.lib)
set (LibGLFW_LIBRARIES glew32s.lib)

# set (MSVC_LIBRARIES "opengl32.lib;User32.lib;Gdi32.lib;Shell32.lib")

message ("Retrieving SDL2-devel-2.0.12-VC.zip from https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip")
file (DOWNLOAD
	https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
	${CMAKE_SOURCE_DIR}/deps/tmp/SDL2-devel-2.0.12-VC.zip
    SHOW_PROGRESS
)

file (ARCHIVE_EXTRACT INPUT 
    ${CMAKE_SOURCE_DIR}/deps/tmp/SDL2-devel-2.0.12-VC.zip
    DESTINATION ${CMAKE_SOURCE_DIR}/deps/)

message ("Retrieving glfw-3.3.2.bin.WIN64.zip from https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip")
file (DOWNLOAD
    https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip
    ${CMAKE_SOURCE_DIR}/deps/tmp/glfw-3.3.2.bin.WIN64.zip
    SHOW_PROGRESS
)

file (ARCHIVE_EXTRACT INPUT
    ${CMAKE_SOURCE_DIR}/deps/tmp/glfw-3.3.2.bin.WIN64.zip
    DESTINATION ${CMAKE_SOURCE_DIR}/deps/)

message ("Retrieving glew-2.1.0-win32.zip from https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download")
file (DOWNLOAD
https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download
    ${CMAKE_SOURCE_DIR}/deps/tmp/glew-2.1.0-win32.zip
    SHOW_PROGRESS
)

file (ARCHIVE_EXTRACT INPUT
    ${CMAKE_SOURCE_DIR}/deps/tmp/glew-2.1.0-win32.zip
    DESTINATION ${CMAKE_SOURCE_DIR}/deps/)

####
# Make setup for the SDL2 include folder required by the
# application. This is kind of a work around to the issue.
# Issue is the folder structure of which the package manager provided
# which does not match downloaded for msvc.
# Package managers provides versioning based of folder name
# as the usually provide older SDL versions. Otherwise use ExternalProject_Add.
####

file (MAKE_DIRECTORY ${LibSDL_DIR}/SDL2)
file (COPY
    ${LibSDL_INCLUDE_DIR}/
    DESTINATION ${LibSDL_DIR}/SDL2
)

file (REMOVE_RECURSE ${LibSDL_INCLUDE_DIR}/)

file (MAKE_DIRECTORY ${LibSDL_INCLUDE_DIR})
file (RENAME
    ${LibSDL_DIR}/SDL2/ ${LibSDL_INCLUDE_DIR}/SDL2/)


# Delete the tmp folder.
file (REMOVE_RECURSE
    ${CMAKE_SOURCE_DIR}/deps/tmp/)

message(${CMAKE_BINARY_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})

# Copy the shared libraries to the binary source.
#file (COPY_FILE 
#    ${LibSDL_LIB_DIR}/SDL2.dll
#    DESTINATION ${CMAKE_BINARY_DIR}/Debug/SDL2.dll)

# Provide the directories for linker.
link_directories (${LibSDL_LIB_DIR} ${LibGLFW_LIB_DIR} ${LibGLEW_LIB_DIR})
