void CatStrings(int sourceACount, char *sourceA, int sourceBCount,
 char *sourceB, int destCount, char *dest)
{
  unsigned int count = 0;
	for (int index = 0; index < sourceACount && count < destCount; index++)
	{
		*dest++ = *sourceA++;
    count++;
	}
	for (int index = 0; index < sourceBCount && count < destCount; index++)
	{
		*dest++ = *sourceB++;
    count++;
	}
	*dest = 0;
}

void CatStrings(int sourceACount, char *sourceA, int sourceBCount,
 char *sourceB, char *dest)
{
  unsigned int count = 0;
	for (int index = 0; index < sourceACount; index++)
	{
		*dest++ = *sourceA++;
    count++;
	}
	for (int index = 0; index < sourceBCount; index++)
	{
		*dest++ = *sourceB++;
    count++;
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

INTERNAL bool StringEquals(char *StringA, char *StringB)
{
	unsigned int StringALength = GetStringLength(StringA);
	unsigned int StringBLength = GetStringLength(StringB);
	if (StringALength == StringBLength)
	{
		for (unsigned int x = 0; x < StringALength; x++)
		{
			if (*StringA++ == *StringB++)
			{
				continue;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

//TODO(Noah): Note, cat strings actually doesn't care what destintation length you pass it
char *MaccisCatStringsUnchecked(char *stringA, char *stringB, char *dest)
{
  CatStrings(GetStringLength(stringA), stringA, GetStringLength(stringB), stringB, dest);
  return dest;
}

char *MaccisCatStrings(char *stringA, char *stringB, char *dest, unsigned int destLength)
{
  CatStrings(GetStringLength(stringA), stringA, GetStringLength(stringB), stringB, destLength, dest);
  return dest;
}

//Here I have a destination length so I can manage overflows for small buffers.
inline void CloneString(char *String, char *Dest, int DestLength)
{
	int Count = 0;
	while ( (*String != 0) && (Count < DestLength) )
	{
		Count++;
		*Dest++ = *String++;
	}
	*Dest = 0;
}

/*
char *BuildFilePath(file_path fpath, char *fileName, char *buffer, int bufferLength)
{
	CatStrings(fpath.length, fpath.path, GetStringLength(fileName), fileName, bufferLength, buffer);
  return buffer;
}
*/
