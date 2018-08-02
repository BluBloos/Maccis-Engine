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

char *BuildFilePath(file_path fpath, char *fileName, char *buffer, int bufferLength)
{
	CatStrings(fpath.length, fpath.path, GetStringLength(fileName), fileName, bufferLength, buffer);
  return buffer;
}