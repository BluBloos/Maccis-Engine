@echo off
pushd build
cl -I ..\include -D GLEW_STATIC -Femaccis -EHsc ..\src\win32_main.cpp /link /LIBPATH:..\lib gdi32.lib opengl32.lib user32.lib glew32s.lib
popd
