#include <Windows.h>
#include <iostream>

#include "glew.h"
#include "gl.h"

#define INTERNAL static
#define PERSISTENT static
#ifdef DEBUG
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

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

struct read_file_result
{
  void *content;
  unsigned int contentSize;
};

void Win32FreeFile(void *memory)
{
  if (memory)
	{
		VirtualFree(memory,0,MEM_RELEASE);
	}
}

read_file_result Win32ReadFile(char *filePath)
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

void CatStrings(int sourceACount, char *sourceA, int sourceBCount,
 char *sourceB, int destCount, char *dest)
{
	for (int index = 0; index < sourceACount; index++)
	{
		*dest++ = *sourceA++;
	}
	for (int index = 0; index < sourceBCount; index++)
	{
		*dest++ = *sourceB++;
	}
	*dest = 0;
}

unsigned int GetStringLength(char *string)
{
	unsigned int count = 0;
	while (*string != 0)
	{
		string++; count++;
	}
	return count;
}

struct relative_path
{
  char path[MAX_PATH];
  unsigned int length;
};


void Win32BuildFilePath(relative_path rpath, char *fileName, char *dest, int destCount)
{
	CatStrings(rpath.length, rpath.path, GetStringLength(fileName), fileName, destCount, dest);
}

relative_path Win32GetRelativePath(relative_path rpath)
{
  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms683197(v=vs.85).aspx
	DWORD pathSize = GetModuleFileNameA(NULL, rpath.path, rpath.length);
  char *buffer = rpath.path;
  for (char *scan = rpath.path;*scan;++scan)
	{
		if (*scan == '\\')
		{
			buffer = scan + 1;
		}
	}
  rpath.length = buffer - rpath.path;
  return rpath;
}

INTERNAL unsigned int CompileShader(unsigned int type, char *shader)
{
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &shader, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    std::cout << "Failed to comile shader!" << std::endl;
    std::cout << message << std::endl;
    glDeleteShader(id);
    return 0;
  }

  return id;
}

INTERNAL unsigned int CreateShaders(char *vertexShader, char *fragmentShader)
{
  unsigned int program = glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program, vs);
  glAttachShader(program, fs);

  //TODO(Noah): Assert if the shader compilation fails

  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
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
      Win32InitOpenGL(windowHandle);

      HDC dc = GetDC(windowHandle);
      RECT rect = {};

      float positions[6] = {
        -0.5f, -0.5f,
         0.0f, 0.5f,
         0.5f, -0.5f
      };

      unsigned int buffer; //create the storage for the generated ID of the buffer
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer); //select the buffer
      glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
      glEnableVertexAttribArray(0);

      //TODO(Noah): Load in the shaders from their respective files and create them
      unsigned int program;
      glUseProgram(program);

      while(globalRunning)
			{
        Win32ProcessMessages();
        GetWindowRect(windowHandle, &rect);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        SwapBuffers(dc);
      }
      ReleaseDC(windowHandle, dc);
    }
  }
}
