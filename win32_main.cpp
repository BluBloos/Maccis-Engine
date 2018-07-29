#include <Windows.h>
#include <stdio.h>
#include "gl.h"

#define INTERNAL static
#define PERSISTENT static

INTERNAL bool globalRunning = true;

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
		}
		break;
		case WM_CLOSE:
		{
			//TODO(Noah): Handle as message to user
			globalRunning = false;
		}
		break;
		default:
		{
			result = DefWindowProc(window,message,wParam,lParam);
		}
		break;
	}

	return result;
}

void Win32ProcessMessages()
{
	MSG message;
	while(PeekMessage(&message,NULL,0,0,PM_REMOVE))
	{
		if(message.message == WM_QUIT)
		{
			globalRunning = false;
		} else
    {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
	}
}

void Win32InitOpenGL(HWND window)
{
  HDC dc = GetDC(window);
  HGLRC glrc = wglCreateContext(dc);
  if(wglMakeCurrent(dc, glrc))
  {
    //NOTE(Noah): everythings good
  }else
  {
    //TODO(Noah): opengl did not initialize
  }
  ReleaseDC(window, dc);
}

int CALLBACK WinMain(HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR cmdLine,
  int showCode)
{
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
      while(globalRunning)
			{
        Win32ProcessMessages();
      }
    } else
    {
      DWORD error = GetLastError();
      char Buffer[256];
      if(FormatMessage(FORMAT_MESSAGE_FROM_STRING, Buffer, 0, 0, NULL, 256, NULL) != 0)
      {
        printf("%s\n", Buffer);
      } else
      {
        printf("Format message failed\n");
      }
    }
  }
}
