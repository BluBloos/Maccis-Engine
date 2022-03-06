@echo off

set maccisDirectory=%cd%

set backend=-D %2
set commonLinkerFlags=/link /LIBPATH:..\..\..\lib -incremental:no
set commonCompilerFlags=-Zi -I ..\..\..\include -I ..\..\..\platform -I ..\..\..\src


mkdir bin
mkdir bin\Windows
mkdir bin\Windows\x86
pushd bin\Windows\x86
del *.pdb
del maccisDirectory.txt
del %maccisDirectory%\config\sourceDLL.txt
echo %maccisDirectory%\>>maccisDirectory.txt
echo %cd%\%1.dll>>%maccisDirectory%\config\sourceDLL.txt
cl %commonCompilerFlags% -D GLEW_STATIC -D DEBUG %backend% -Femaccis -EHsc ..\..\..\platform\win32_main.cpp %commonLinkerFlags% gdi32.lib opengl32.lib user32.lib glew32s.lib kernel32.lib
cl %commonCompilerFlags% -Fefont ..\..\..\platform\win32_font.cpp %commonLinkerFlags% gdi32.lib user32.lib
cl %commonCompilerFlags% 2-Feobj ..\..\..\platform\win32_obj.cpp %commonLinkerFlags% user32.lib
cl %commonCompilerFlags% ..\..\..\src\%1.cpp -LD %commonLinkerFlags% -PDB:%1_%random%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameInit -EXPORT:GameClose
popd
