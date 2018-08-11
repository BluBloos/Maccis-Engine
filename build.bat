@echo off

set maccisDirectory=%cd%

set commonLinkerFlags=/link /LIBPATH:..\..\..\lib -incremental:no
set commonCompilerFlags=-Zi -I ..\..\..\include -I ..\..\..\platform -I ..\..\..\src

pushd bin\Windows\x86
del *.pdb
del maccisDirectory.txt
echo %maccisDirectory%\>>maccisDirectory.txt
cl %commonCompilerFlags% -D GLEW_STATIC -D DEBUG -Femaccis -EHsc ..\..\..\platform\win32_main.cpp %commonLinkerFlags% gdi32.lib opengl32.lib user32.lib glew32s.lib kernel32.lib
cl %commonCompilerFlags% -Fefont ..\..\..\platform\win32_font.cpp %commonLinkerFlags% gdi32.lib user32.lib
cl %commonCompilerFlags% -Feobj ..\..\..\platform\win32_obj.cpp %commonLinkerFlags% user32.lib
popd
