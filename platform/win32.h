struct win32_timing
{
  LARGE_INTEGER start, end;
  LONGLONG perfCountFrequency;
};

#define WIN32_CALL(code) if (code == NULL){Win32PrintError();}else

void Win32PrintError()
{
  DWORD err = GetLastError();
  char buffer[MAX_PATH];
  if( FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    err,
    MAKELANGID(LANG_ENGLISH, 0x01),
    (LPSTR)buffer,
    MAX_PATH,
    NULL) != 0 )
  {
    printf("ERROR: %s", buffer);
  } else {
    printf("Failed to format error message!\n");
  }
}

//NOTE(Noah): Another beautiful inline function. I kinda love these functions.
//They are very funcitonal.
inline LARGE_INTEGER Win32GetWallClock()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result;
}

//note this is a functional function. I think functional functions should be inline.
//That I deside as of now.
inline float Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end, LONGLONG perfCountFrequency64)
{
	float result = 1.0f * (end.QuadPart - start.QuadPart) / perfCountFrequency64;
	return result;
}

//TODO(Noah): I dislike this function and its various dependencies!
INTERNAL void Win32GetRelativePath(char *dest, unsigned int length)
{
  char stringBuffer[MAX_PATH];
  char stringBuffer2[MAX_PATH];
  //https://msdn.microsoft.com/en-us/library/windows/desktop/ms683197(v=vs.85).aspx
	DWORD pathSize = GetModuleFileNameA(NULL, stringBuffer, length);
  char *buffer = stringBuffer;
  char *buffer2 = stringBuffer;
  for (char *scan = stringBuffer;*scan;++scan)
	{
		if (*scan == '\\')
		{
      buffer2 = buffer;
			buffer = scan + 1;
		}
	}
  length = buffer - stringBuffer;
  MaccisCatStrings(stringBuffer, "garbage", stringBuffer2, length);
  read_file_result fileResult = Win32ReadFile(MaccisCatStringsUnchecked(stringBuffer2, "maccisDirectory.txt", stringBuffer));
  if(fileResult.content)
  {
    //NOTE(Noah): There is minus 2 here to remove the \r\n characters!
    CloneString((char *)fileResult.content, dest, fileResult.contentSize - 2);
  } else
  {
    //TODO(Noah): do some logging since we could not find the maccis directory file!
  }
}
