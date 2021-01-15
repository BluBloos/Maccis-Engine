# PongGL 🏓
Attempted game engine, but ultimately an overengineered version of pong. The project was written in C/C++, with minimal to no library usage, and it uses the OpenGL graphics API.

## Features
As the project description implies, on the surface this may seem like a video game, but the source code was written with the intention of making generic games. In fact, there is another "game" written using the "engine". See the build steps below for instructions on compiling and running this demo. 

The engine has the following features:
- BORIS 

## Build Steps
This project uses the visual studio compiler, so you will need to install <a href="https://visualstudio.microsoft.com/vs/">Visual Studio</a> if you haven't already. At the time of writing, the latest version is 2019, so I cannot guarentee proper building for any subsequent versions. Also note that if your visual studio version is different than 2019, you will need to change *shell.bat* accordingly.   

After installing visual studio, clone this project and run the following commands. This will set up the visual studio compiler for use on the command line via the "cl" command. The *build.bat* script takes care of compiling the platform layer, the engine layer, and the game.dll
```
$ shell.bat
$ build.bat pong GL_BACKEND
```
Finally, to run the game, run the following commands,  
```
$ cd  bin/Windows/x86
$ maccis.exe
```

### Building Other Games
The parameters of the build script are as follows,
```
$ build.bat <game_source> <backend> 
```
The game source is a single .cpp file with the source code for the game, and the backend is the graphics API. GL_BACKEND is the only supported API. So, to build the other example, run the following command,
```
$ build.bat spinning_monkey GL_BACKEND 
```
To run the game, it is the same sequence of commands as with the pong example
```
$ cd  bin/Windows/x86
$ maccis.exe
```

## Controls
### Pong
For pong, the left paddle moves with the W and S keys, and the right paddle with the up and down arrow keys. 
### Spinning Monkey
For the spinning monkey, control the camera with WASD. Use space and shift for moving the camera up and down, respectively. Click and drag the mouse to change where the camera is looking.

