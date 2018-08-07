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
