#include <Windows.h>
#include <glew.h>
#include <gl.h>

#include <maccis.h> //global to everything
#include <platform.h>
#include <maccis_user_input.h>
#include <maccis_math.h>

#include <backend.h>

//rendering api wrappers which are a service to the engine
#ifdef GL_BACKEND
#include <gl_backend.cpp>
#endif

#ifdef VULKAN_BACKEND
#include <vulkan_backend.cpp>
#endif

#ifdef DX12_BACKEND
#include <d12_backend.cpp>
#endif

#include <engine.cpp> //the engine is a service to the platform

//windows stuff
#include <maccis_system.h>
#include <win32_file_io.cpp>
#include <win32.h>

struct win32_game_code
{
  game_init *GameInit;
  game_update_and_render *GameUpdateAndRender;
  game_close *GameClose;
  bool isValid;
  FILETIME lastWriteTime;
  HMODULE gameCodeDLL;
};

#define MAX_CONSOLE_LINES 500
typedef BOOL WINAPI wgl_swap_interval_ext(int interval);

//setup global variables
INTERNAL bool globalRunning = true;
INTERNAL wgl_swap_interval_ext *wglSwapInterval;
INTERNAL user_input globalUserInput = {};
INTERNAL win32_timing globalTimer = {};

INTERNAL void Win32OpenConsole()
{
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

  // allocate a console for this app
  //https://docs.microsoft.com/en-us/windows/console/allocconsole
  AllocConsole();
  SetConsoleTitleA("Maccis Console");

  // set the screen buffer to be big enough to let us scroll text
  //https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  // redirect unbuffered STDOUT to the console
  //https://msdn.microsoft.com/en-us/library/bdts1c9x.aspx
  //https://msdn.microsoft.com/en-us/library/88k7d7a7.aspx
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT); //convert windows handle to c runtime handle
  fp = _fdopen( hConHandle, "w" );
  *stdout = *fp;
  //setvbuf( stdout, NULL, _IONBF, 0 ); //associate no buffer
  freopen_s( &fp, "CONOUT$", "w", stdout);

  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  *stderr = *fp;
  //setvbuf( stdout, NULL, _IONBF, 0 ); //associate no buffer
  freopen_s(&fp, "CONOUT$" , "w" , stderr);
}

INTERNAL void Win32PrepareInput()
{
  for (unsigned int i = 0; i < MAX_KEY_STATES; i++ )
  {
    globalUserInput.keyStates[i].halfTransitionCount = 0;
  }
}

INTERNAL inline void Win32ProcessKeyboardKey(unsigned int key, bool down)
{
  if (globalUserInput.keyStates[key].endedDown != down)
  {
    globalUserInput.keyStates[key].halfTransitionCount++;
    globalUserInput.keyStates[key].endedDown = down;
  }
}

INTERNAL void Win32ProccessKeyboardMessage(unsigned int vkCode, bool down)
{
  switch (vkCode)
  {
    case 'W':
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_W, down);
    } break;
    case 'A':
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_A, down);
    } break;
    case 'S':
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_S, down);
    } break;
    case 'D':
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_D, down);
    } break;
    case VK_SPACE:
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_SPACE, down);
    } break;
    case VK_ESCAPE:
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_ESCAPE, down);
    } break;
    case VK_SHIFT:
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_SHIFT, down);
    } break;
    case VK_RETURN: //enter key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_ENTER, down);
    } break;
    case VK_CONTROL:
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_CONTROL, down);
    } break;
    case VK_MENU: //alt key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_ALT, down);
    } break;
    case VK_LEFT: //left arrow key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_LEFT, down);
    } break;
    case VK_RIGHT: //right arrow key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_RIGHT, down);
    } break;
    case VK_UP: //up arrow key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_UP, down);
    } break;
    case VK_DOWN: //down arrow key
    {
      Win32ProcessKeyboardKey(MACCIS_KEY_DOWN, down);
    } break;
  }
}

LRESULT CALLBACK Win32WindowProc(HWND window,
  UINT message,
  WPARAM wParam,
  LPARAM lParam)
{
  LRESULT result = 0;

  switch(message)
	{
		case WM_DESTROY:
		{
			//TODO(Noah): Handle as error
			globalRunning = false;
		} break;
		case WM_CLOSE:
		{
			//TODO(Noah): Handle as message to user
			globalRunning = false;
		} break;

    //keyboard messages
    case WM_KEYUP:
    {
      unsigned int vkCode = wParam;
      Win32ProccessKeyboardMessage(vkCode, false);
    } break;
    case WM_KEYDOWN:
    {
      unsigned int vkCode = wParam;
      Win32ProccessKeyboardMessage(vkCode, true);
    } break;

    //left mouse button
    case WM_LBUTTONDOWN:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_LEFT, true);
    } break;
    case WM_LBUTTONUP:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_LEFT, false);
    } break;

    //right mouse button
    case WM_RBUTTONDOWN:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_RIGHT, true);
    } break;
    case WM_RBUTTONUP:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_RIGHT, false);
    } break;

    //middle mouse button
    case WM_MBUTTONDOWN:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_MIDDLE, true);
    } break;
    case WM_MBUTTONUP:
    {
      Win32ProcessKeyboardKey(MACCIS_MOUSE_MIDDLE, false);
    } break;

    //mouse movement
    case WM_MOUSEMOVE:
    {
      int x = (int)lParam & 0x0000FFFF;
      int y = ((int)lParam & 0xFFFF0000) >> 16;
      globalUserInput.mouseX = x;
      globalUserInput.mouseY = y;
    } break;

		default:
		{
			result = DefWindowProc(window,message,wParam,lParam);
		} break;
	}

	return result;
}

INTERNAL void Win32ProcessMessages()
{
	MSG message;
	while(PeekMessage(&message,NULL,0,0,PM_REMOVE))
	{
    switch (message.message)
    {
      case WM_QUIT:
      {
        globalRunning = false;
      } break;
      default:
      {
        TranslateMessage(&message);
        DispatchMessage(&message);
      } break;
    }
	}

}

INTERNAL void Win32InitOpenGL(HWND window)
{
  HDC dc = GetDC(window);

  PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
  desiredPixelFormat.nSize = sizeof(desiredPixelFormat);
  desiredPixelFormat.nVersion = 1;
  desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
  desiredPixelFormat.cColorBits = 32;
  desiredPixelFormat.cAlphaBits = 8;
  desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

  int suggestedPixelFormatIndex = ChoosePixelFormat(dc, &desiredPixelFormat);
  PIXELFORMATDESCRIPTOR suggestedPixelFormat = {};
  DescribePixelFormat(dc, suggestedPixelFormatIndex,
    sizeof(suggestedPixelFormat), &suggestedPixelFormat);
  SetPixelFormat(dc, suggestedPixelFormatIndex, &suggestedPixelFormat);

  HGLRC glrc = wglCreateContext(dc);
  if(wglMakeCurrent(dc, glrc))
  {
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
      //TODO(Noah): something is seriously wrong
    }

    //setup VSYNC, even though it's on by default :)
    wglSwapInterval = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
    if(wglSwapInterval)
    {
      wglSwapInterval(1);
    }

    //enable depth buffering
    glEnable(GL_DEPTH_TEST);
  }else
  {
    //TODO(Noah): opengl did not initialize
  }
  ReleaseDC(window, dc);
}

PLATFORM_GET_CLOCK(Win32StartClock)
{
  globalTimer.start = Win32GetWallClock();
}

PLATFORM_GET_CLOCK(Win32EndClock)
{
  globalTimer.end = Win32GetWallClock();
}

PLATFORM_GET_DELTA_TIME(Win32GetClockDeltaTime)
{
  return Win32GetSecondsElapsed(globalTimer.start, globalTimer.end, globalTimer.perfCountFrequency);
}

PLATFORM_LOG(Win32Log)
{
  printf("%s", message);
}

inline FILETIME Win32GetLastWriteTime(char *fileName)
{
	FILETIME lastFileWrite = {};
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFileA(fileName, &findData);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(findHandle);
		lastFileWrite = findData.ftLastWriteTime;
	}
	return lastFileWrite;
}

INTERNAL void Win32UnloadGameCode(win32_game_code *gameCode)
{
	if (gameCode->gameCodeDLL)
	{
		FreeLibrary(gameCode->gameCodeDLL);
	}
	gameCode->isValid = false;
	gameCode->GameUpdateAndRender = NULL;
	gameCode->GameClose = NULL;
  gameCode->GameInit = NULL;
}

INTERNAL win32_game_code Win32LoadGameCode(char *sourceDLLName, char *tempDLLName)
{
	win32_game_code result = {};

	result.lastWriteTime = Win32GetLastWriteTime(sourceDLLName);

	CopyFile(sourceDLLName, tempDLLName, FALSE);
	result.gameCodeDLL = LoadLibraryA(tempDLLName);

	if(result.gameCodeDLL)
	{
		result.GameUpdateAndRender = (game_update_and_render *)GetProcAddress(result.gameCodeDLL,"GameUpdateAndRender");
		result.GameInit = (game_init *)GetProcAddress(result.gameCodeDLL,"GameInit");
    result.GameClose = (game_close *)GetProcAddress(result.gameCodeDLL, "GameClose");
		result.isValid = result.GameUpdateAndRender && result.GameInit && result.GameClose;
	}

	if (!result.isValid)
	{
		result.GameUpdateAndRender = NULL;
		result.GameClose = NULL;
    result.GameInit = NULL;
	}

	return result;
}

INTERNAL game_code Win32GameCodeToGameCode(win32_game_code win32GameCode)
{
  game_code gameCode = {};
  gameCode.isValid = win32GameCode.isValid;
  gameCode.GameUpdateAndRender = win32GameCode.GameUpdateAndRender;
  gameCode.GameClose = win32GameCode.GameClose;
  gameCode.GameInit = win32GameCode.GameInit;
  return gameCode;
}

int CALLBACK WinMain(HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR cmdLine,
  int showCode)
{
  Win32StartClock();
  //initialize the console
  Win32OpenConsole();
  printf("Maccis-Engine version %d!\n", 1);
  fprintf(stdout, "stdout initialized\n");
  fprintf(stderr, "stderr initialized\n");

  //build the relative path
  char win32FilePath[MAX_PATH];
  Win32GetRelativePath(win32FilePath, MAX_PATH);

  //set up the win32_timer
  LARGE_INTEGER perfCountFrequency;
	QueryPerformanceFrequency(&perfCountFrequency);
	globalTimer.perfCountFrequency = perfCountFrequency.QuadPart;

  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms633576
  WNDCLASS windowClass = {};
  windowClass.style = CS_VREDRAW|CS_HREDRAW; //Set window to redraw after being resized
  windowClass.lpfnWndProc = Win32WindowProc; //Set callback
  windowClass.hInstance = instance; //Set handle to instance that contains window procedure
  windowClass.hCursor = LoadCursor(0, IDC_ARROW);
  windowClass.lpszClassName = "Maccis Window Class"; //Specify the window class name

  if(RegisterClass(&windowClass))
	{
    //https://msdn.microsoft.com/en-us/library/windows/desktop/ms632680
		HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "Maccis",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_MAXIMIZE, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
			instance, NULL);

    if(windowHandle)
    {
      Win32InitOpenGL(windowHandle);

      HDC dc = GetDC(windowHandle);
      RECT rect = {};
      GetWindowRect(windowHandle, &rect);

      //create the game codes
      win32_game_code gameCode = {};
      game_code engineGameCode = {};

      engine_memory engineMemory = {};
      engineMemory.storageSize = MB(64);
      engineMemory.storage = VirtualAlloc(0, engineMemory.storageSize, MEM_COMMIT, PAGE_READWRITE);

      engineMemory.maccisDirectory = win32FilePath;
      engineMemory.ReadFile = Win32ReadFile;
      engineMemory.FreeFile = Win32FreeFile;
      engineMemory.StartClock = Win32StartClock;
      engineMemory.EndClock = Win32EndClock;
      engineMemory.GetClockDeltaTime = Win32GetClockDeltaTime;
      engineMemory.Log = Win32Log;

      engine_state *engineState = (engine_state *)engineMemory.storage;
      engineState->memoryArena.init((char *)engineMemory.storage + sizeof(engine_state), engineMemory.storageSize - sizeof(engine_state));

      //load in the source DLL name
      char sourceDLLName[MAX_PATH];
      char tempDLLName[MAX_PATH];
      char stringBuffer[MAX_PATH];
      read_file_result fileResult = Win32ReadFile(MaccisCatStringsUnchecked(win32FilePath, "config\\sourceDLL.txt", stringBuffer));
      if(fileResult.content)
      {
        //NOTE(Noah): There is minus 2 here to remove the \r\n characters!
        CloneString((char *)fileResult.content, sourceDLLName, fileResult.contentSize - 2);
        MaccisCatStringsUnchecked(sourceDLLName, ".temp", tempDLLName);
      } else
      {
        //TODO(Noah): do some logging since we could not find the maccis directory file!
      }

      gameCode = Win32LoadGameCode(sourceDLLName, tempDLLName);
      engineGameCode = Win32GameCodeToGameCode(gameCode);

      Init(engineGameCode, engineMemory, rect.right - rect.left, rect.bottom - rect.top);

      int lastMouseX, lastMouseY;

      while(globalRunning)
			{
        FILETIME newDLLWriteTime = Win32GetLastWriteTime(sourceDLLName);
				if (CompareFileTime(&newDLLWriteTime, &gameCode.lastWriteTime))
				{
					Win32UnloadGameCode(&gameCode);
          //NOTE(Noah): I think we need the temporary DLL name to enable hot reloading. We can't rebuild the DLL if it's already loaded!
          //We got to clone it!
          gameCode = Win32LoadGameCode(sourceDLLName, tempDLLName);
          engineGameCode = Win32GameCodeToGameCode(gameCode);
				}

        lastMouseX = globalUserInput.mouseX;
        lastMouseY = globalUserInput.mouseY;

        Win32ProcessMessages();

        globalUserInput.mouseDX = globalUserInput.mouseX - lastMouseX;
        globalUserInput.mouseDY = globalUserInput.mouseY - lastMouseY;

        Update(engineGameCode, engineMemory, globalUserInput);
        SwapBuffers(dc);
        Win32PrepareInput();
      }

      Clean(engineGameCode, engineMemory);
      ReleaseDC(windowHandle, dc);
      VirtualFree(engineMemory.storage, engineMemory.storageSize, MEM_RELEASE);
    }
  }
}
