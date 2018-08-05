@echo off
set commonLinkerFlags=/link /LIBPATH:..\lib -incremental:no
set commonCompilerFlags=-Zi -I ..\include -I ..\engine -I ..\io -I ..\platform -I ..\renderer -I ..\utility
pushd build
del *.pdb
cl %commonCompilerFlags% -D GLEW_STATIC -D DEBUG -Femaccis -EHsc ..\platform\win32_main.cpp %commonLinkerFlags% gdi32.lib opengl32.lib user32.lib glew32s.lib kernel32.lib
cl %commonCompilerFlags% -Feconsole ..\platform\win32_utility.cpp %commonLinkerFlags% user32.lib
popd
