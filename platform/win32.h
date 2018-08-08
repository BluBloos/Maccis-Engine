struct win32_timing
{
  LARGE_INTEGER start, end;
  LONGLONG perfCountFrequency;
};

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

INTERNAL void Win32GetRelativePath(char *dest, unsigned int length)
{
  char stringBuffer[MAX_PATH];
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
  length = buffer2 - stringBuffer;
  MaccisCatStrings(stringBuffer, "garbage", dest, length);
}
