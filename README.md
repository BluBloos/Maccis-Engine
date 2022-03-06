<img src="https://i.gyazo.com/427ac0168a87c73c9768de86f94d5d0b.gif" />

# Maccis-Engine 🥜

A small game-engine written in C and OpenGL, with minimal-to-no library usage. The project includes two demos: pong, and a spinning monkey head.

# Steps for Building
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

## Building Novel Demos
The parameters of the build script are as follows,

```
$ build.bat <gameSourceWithoutExtension> <backend> 
```

gameSource corresponds to a single .cpp file containing the source code for the game, and the backend is the graphics API. GL_BACKEND is the only supported API. To build the only other example, run the following

```
$ build.bat spinning_monkey GL_BACKEND 
```

To run the game, it is the same sequence of commands as with the pong example

```
$ cd  bin/Windows/x86
$ maccis.exe
```

## Demo Controls
### Pong
For pong, the left paddle moves with the W and S keys, and the right paddle with the up and down arrow keys. 
### Spinning Monkey
For the spinning monkey demo, control the camera with WASD. Use space and shift for moving the camera up and down, respectively. Click and drag the mouse to change where the camera is looking.

# Features

The engine has the following features:
- File IO
- Timing
- 3D model loading and rendering
- Custom assets
- 2D batch rendering
- True type font support
- Orthographic and projection cameras
- Custom shaders
- Texture support



