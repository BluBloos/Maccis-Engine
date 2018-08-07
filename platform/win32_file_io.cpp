PLATFORM_FREE_FILE(Win32FreeFile)
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

PLATFORM_WRITE_FILE(Win32WriteFile)
{
	bool result = false;

	HANDLE fileHandle = CreateFileA(fileName, GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle,memory,memorySize,&bytesWritten,0))
		{
			//File read succesfully
			result = ((int)bytesWritten == memorySize);
		}
		else
		{
			//could not write to file, do some logging!
		}
		CloseHandle(fileHandle);
	}
	else
	{
		//Could not open file handle do some logging!
	}
	return result;
}
