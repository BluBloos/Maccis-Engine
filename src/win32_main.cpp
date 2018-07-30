#include <Windows.h>
#include <iostream>
#include <glew.h>
#include <gl.h>

#include <engine.cpp>

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
  }else
  {
    //TODO(Noah): opengl did not initialize
  }
  ReleaseDC(window, dc);
}

void Win32FreeFile(void *memory)
{
  if (memory)
	{
		VirtualFree(memory,0,MEM_RELEASE);
	}
}

PLATFORM_READ_FILE(Win32ReadFile)
{
  read_file_result fileResult = {};
	unsigned int fileSize32;
  //https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
	HANDLE fileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, 0, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize64;
    //https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-getfilesizeex
		if (GetFileSizeEx(fileHandle, &fileSize64))
		{
			Assert(fileSize64.QuadPart <= 0xFFFFFFF);
			fileSize32 = (unsigned int)fileSize64.QuadPart;
			fileResult.content = VirtualAlloc(NULL, fileSize32, MEM_COMMIT, PAGE_READWRITE);
			if (fileResult.content)
			{
				DWORD bytesRead;
        //https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-readfile
				if (!ReadFile(fileHandle, fileResult.content, fileSize32,
					&bytesRead, NULL) || fileSize32 != (unsigned int)bytesRead)
				{
					Win32FreeFile(fileResult.content);
					fileResult.content = NULL;
          return fileResult;
				}
        fileResult.contentSize = fileSize32;
			}
		}
		CloseHandle(fileHandle);
	}
	return fileResult;
}

file_path Win32GetRelativePath(file_path rpath)
{
  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms683197(v=vs.85).aspx
	DWORD pathSize = GetModuleFileNameA(NULL, rpath.path, rpath.length);
  char *buffer = rpath.path;
  char *buffer2 = rpath.path;
  for (char *scan = rpath.path;*scan;++scan)
	{
		if (*scan == '\\')
		{
      buffer2 = buffer;
			buffer = scan + 1;
		}
	}
  rpath.length = buffer2 - rpath.path;
  return rpath;
}

int CALLBACK WinMain(HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR cmdLine,
  int showCode)
{
  char stringBuffer[256];
  file_path filePath = {}; filePath.length = MAX_PATH;
  filePath = Win32GetRelativePath(filePath);

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

      engine_memory engineMemory = {};
      engineMemory.maccisDirectory = filePath;
      engineMemory.ReadFile = Win32ReadFile;

      Init(engineMemory);

      while(globalRunning)
			{
        Win32ProcessMessages();
        //GetWindowRect(windowHandle, &rect);

        Update(engineMemory);

        SwapBuffers(dc);
      }
      ReleaseDC(windowHandle, dc);
    }
  }
}
