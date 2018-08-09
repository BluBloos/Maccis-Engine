#include <windows.h>
#include <stdio.h>

#include <maccis.h>
#include <maccis_math.h>
#include <maccis_strings.h>
#include <win32.h>
#include <platform.h>
#include <maccis_file_io.h>
#include <maccis_asset.h>

//TODO(Noah): Export this funciton to outside of this file
void DrawBitmapUnchecked(loaded_bitmap bitmap, unsigned int *pixelPointer, unsigned int pixelPitch)
{
  unsigned int *destRow = pixelPointer;
  unsigned int *sourceRow = bitmap.pixelPointer;
  for (unsigned int y = 0; y < bitmap.height; y++)
  {
    unsigned int *source = sourceRow;
    unsigned int *dest = destRow;
    for (unsigned int x =0; x < bitmap.width; x++)
    {
      *dest++ = *source++;
    }
    sourceRow += bitmap.width;
    destRow += pixelPitch;
  }
}

#include <file_io.cpp>
#include <asset.cpp>
#include <win32_fonts.cpp>
#include <win32_file_io.cpp>

INTERNAL bool globalRunning = true;

int main()
{
  printf("Welcome to the Maccis-Engine command line interface! Type 'help' for help\n");
  char stringBuffer[256];

  unsigned int storageSize = MB(64);
  void *storage = VirtualAlloc(0, storageSize, MEM_COMMIT, PAGE_READWRITE);

  memory_arena arena = {};
  arena.init((char *)storage, storageSize);

	char win32FilePath[MAX_PATH];
  Win32GetRelativePath(win32FilePath, MAX_PATH);

  while(globalRunning)
  {
    scanf("%s", stringBuffer);
    if ( StringEquals(stringBuffer, "help") )
    {

    }
    else if ( StringEquals(stringBuffer, "font") )
    {
      printf("Enter a font name\n");
      scanf("%s", stringBuffer);

      //build the font asset
      loaded_asset asset = BuildFontAsset(Win32ReadFile, Win32FreeFile, Win32WriteFile,
        &arena, stringBuffer, 60.0f);

      printf("Enter a file name to save to\n");
      scanf("%s", stringBuffer);

      WriteAsset(Win32WriteFile, &arena, &asset, stringBuffer);
    }
		else if (StringEquals(stringBuffer, "obj"))
		{
			printf("Enter the file path of the obj to preparse\n");
			scanf("%s", stringBuffer);

			//preparse the obj file
			raw_model model = LoadOBJ(&arena, win32FilePath, stringBuffer);

			//write the raw_model to the asset file
			loaded_asset asset = {};
			PushRawModelToAsset(model, &asset, &arena);

			//write the asset
			printf("Enter a file name to save to\n");
			scanf("%s", stringBuffer);
			WriteAsset(Win32WriteFile, &arena, &asset, stringBuffer);
		} else
    {
      printf("Invalid command!\n");
    }
  }
}
