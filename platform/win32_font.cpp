#include <windows.h>
#include <stdio.h>
#include <maccis.h>
#include <platform.h>
#include <win32_file_io.cpp>
#include <maccis_strings.h>
#include <maccis_file_io.h>
#include <win32.h>
#include <maccis_asset.h>
#include <maccis_math.h>
#include <asset.cpp>
#include <file_io.cpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define DONT_USE_WINDOWS_FONTS 0

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

#if DONT_USE_WINDOWS_FONTS
INTERNAL loaded_bitmap BuildCharacterBitmap(read_file_result fontFile, char character, float pixelHeight,
  memory_arena *arena)
{
  loaded_bitmap result = {};

  stbtt_fontinfo font;
  if(!stbtt_InitFont(&font, (unsigned char *)fontFile.content, stbtt_GetFontOffsetForIndex((unsigned char *)fontFile.content, 0)))
  {
    //TODO(Noah): logging since it failed to initialize the font
  }

  int width, height, xoffset, yoffset;
  unsigned char *monoBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, pixelHeight), character,
    &width, &height, &xoffset, &yoffset);

  //Allocate a bitmap so we can fill it!
  result = MakeEmptyBitmap(arena, width, height);

  unsigned char *source = monoBitmap;
  unsigned int *destRow = result.pixelPointer + (height - 1) * width;
  for (unsigned int y = 0; y < height; y++)
  {
    unsigned int *dest = (unsigned int *)destRow;
    for (unsigned int x = 0; x < width; x++)
    {
      unsigned char alpha = *source++;
      *dest++ = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    }
    destRow -= width;
  }

  //This function will free an 8bpp bitmap
  stbtt_FreeBitmap(monoBitmap, 0);

  //finally return the bitmap which was so painstakingly crafted!
  return result;
}
#else
INTERNAL loaded_bitmap BuildCharacterBitmap(char *path, char *fontName,
  char character, float pixelHeight, memory_arena *arena)
{
  loaded_bitmap result = {};

  PERSISTENT HDC deviceContext = 0;
  if (!deviceContext)
  {
    int addedFonts = AddFontResourceEx(path, FR_PRIVATE, 0);
    //printf("added fonts: %d\n", addedFonts);
    HFONT font = CreateFontA((int)pixelHeight, 0, 0, 0,
      FW_NORMAL, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
      DEFAULT_PITCH | FF_DONTCARE, fontName);
    deviceContext = CreateCompatibleDC(NULL);
    WIN32_CALL(deviceContext)
    {
      HBITMAP bitmap = CreateCompatibleBitmap(deviceContext, 1024, 1024);
      SelectObject(deviceContext, bitmap);
      SelectObject(deviceContext, font);

      TEXTMETRIC textMetric;
      GetTextMetrics(deviceContext, &textMetric);
    }
  }

  SIZE size = {};
  wchar_t cheesePoint = (wchar_t)character;

  GetTextExtentPoint32W(deviceContext, &cheesePoint, 1, &size);

  int width = size.cx;
  int height = size.cy;

  SetTextColor(deviceContext, RGB(255, 255, 255));
  SetBkColor(deviceContext, RGB(0,0,0));
  TextOutW(deviceContext, 0, 0, &cheesePoint, 1);

  int minX = 10000;
  int maxX = -10000;
  int minY = 10000;
  int maxY = -10000;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      COLORREF pixel = GetPixel(deviceContext, x, y);
      if (pixel & 0xFF) //there is pixel!
      {
        if (x > maxX)
        {
          maxX = x;
        } else if(x < minX)
        {
          minX = x;
        }

        if (y > maxY)
        {
          maxY = y;
        } else if(y < minY)
        {
          minY = y;
        }
      }
    }
  }

  width = maxX - minX + 1;
  height = maxY - minY + 1;

  //Allocate a bitmap so we can fill it!
  result = MakeEmptyBitmap(arena, width, height);

  unsigned int *destRow = result.pixelPointer + (height - 1) * width;
  for (unsigned int y = minY; y < maxY + 1; y++)
  {
    unsigned int *dest = (unsigned int *)destRow;
    for (unsigned int x = minX; x < maxX + 1; x++)
    {
      COLORREF pixel = GetPixel(deviceContext, x, y);
      unsigned char alpha =  (unsigned char)(pixel & 0xFF);
      *dest++ = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    }
    destRow -= width;
  }

  //finally return the bitmap which was so painstakingly crafted!
  return result;
}
#endif

#include <temp.cpp>

INTERNAL loaded_asset BuildFontAsset(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
  memory_arena *arena, char * font, float pixelHeight)
{
  loaded_asset asset = {};

  char stringBuffer[MAX_PATH];
  char stringBuffer2[MAX_PATH];
  MaccisCatStringsUnchecked("C:\\Windows\\Fonts\\", font, stringBuffer);
  MaccisCatStringsUnchecked(stringBuffer, ".ttf", stringBuffer2);
  read_file_result fileResult = ReadFile(stringBuffer2);

  loaded_bitmap characters[127 - 33] = {};
  unsigned int index = 0;

  for (unsigned int i = 33; i < 127; i++)
  {
    index = i - 33;
    //TODO(Implement freeing on the memory_arena so that we can destroy the unused bitmaps!)
    #if DONT_USE_WINDOWS_FONTS
    characters[index] = BuildCharacterBitmap(fileResult, i, pixelHeight, arena);
    #else
    characters[index] = BuildCharacterBitmap(stringBuffer2, font, i, pixelHeight, arena);
    #endif
  }

  unsigned int pixelPitch = 1080;
  unsigned int tallestBitmap = 0;
  unsigned int currentLine = 0;
  unsigned int currentX = 0;
  unsigned int totalHeight = 0;
  for(unsigned int i = 0; i < 127 - 33; i++)
  {
    currentX += characters[i].width + PADDING;
    if (currentX > pixelPitch)
    {
      currentX = characters[i].width + PADDING; //advance the x of the new line to be the bitmap wrapped down to it
      currentLine += 1; //go to next line
      totalHeight += tallestBitmap; //add the height of the last line to the total height
      tallestBitmap = characters[i].height + PADDING; //set the talles bitmap to the height of the wrapped bitmap
    }
    if(tallestBitmap < characters[i].height + PADDING)
    {
      tallestBitmap = characters[i].height + PADDING;
    }
  }
  totalHeight += tallestBitmap;

  //setup font atlas bitmap and character descriptors
  loaded_bitmap atlas = {};
  atlas.width = pixelPitch;
  atlas.height = totalHeight;
  atlas.pixelPointer = (unsigned int *)arena->push(pixelPitch * totalHeight * sizeof(unsigned int));
  character_desriptor descriptors[127 - 33] = {};

  //parse through each bitmap and clone it into the font atlas and write the texture coordinates
  tallestBitmap = 0;
  currentLine = 0;
  currentX = 0;
  totalHeight = 0;
  unsigned int *pixelPointer = atlas.pixelPointer;
  for (unsigned int i = 0; i < 127 - 33; i++)
  {
    loaded_bitmap currentBitmap = characters[i];
    character_desriptor *descriptor = &descriptors[i];
    currentX += currentBitmap.width + PADDING;

    if (currentX > pixelPitch)
    {
      //TODO(Noah): add padding!
      pixelPointer += pixelPitch - (currentX - currentBitmap.width - PADDING); //advance the pixel pointer to the start of the next line
      pixelPointer += pixelPitch * (tallestBitmap + PADDING - 1); //advance the pixel pointer by tallestBitmap - 1 lines, which advances us to the next "line"
      DrawBitmapUnchecked(currentBitmap, pixelPointer, atlas.width);

      currentX = currentBitmap.width + PADDING; //advance the x of the new line to be the bitmap wrapped down to it
      currentLine += 1; //go to next lines
      totalHeight += tallestBitmap; //add the height of the last line to the total height
      tallestBitmap = currentBitmap.height + PADDING; //set the talles bitmap to the height of the wrapped bitmap

      vec2 topLeftVertex = NewVec2(0.0f, (float)totalHeight + currentBitmap.height);
      //vertex 0
      descriptor->textureCoordinate[0] = 0.0f;
      descriptor->textureCoordinate[1] = (topLeftVertex.y - currentBitmap.height)  / atlas.height;
      //vertex 1
      descriptor->textureCoordinate[2] = (float)currentBitmap.width / atlas.width;
      descriptor->textureCoordinate[3] = (topLeftVertex.y - currentBitmap.height)  / atlas.height;
      //vertex 2
      descriptor->textureCoordinate[4] = (float)currentBitmap.width / atlas.width;
      descriptor->textureCoordinate[5] = topLeftVertex.y / atlas.height;
      //vertex 3
      descriptor->textureCoordinate[6] = 0.0f;
      descriptor->textureCoordinate[7] = topLeftVertex.y / atlas.height;
    }
    else
    {
      DrawBitmapUnchecked(currentBitmap, pixelPointer, atlas.width);

      vec2 topLeftVertex = NewVec2((float)currentX - currentBitmap.width - PADDING, (float)totalHeight + currentBitmap.height);
      //vertex 0
      descriptor->textureCoordinate[0] = topLeftVertex.x / atlas.width;
      descriptor->textureCoordinate[1] = (topLeftVertex.y - currentBitmap.height) / atlas.height;
      //vertex 1
      descriptor->textureCoordinate[2] = (topLeftVertex.x + currentBitmap.width) / atlas.width;
      descriptor->textureCoordinate[3] = (topLeftVertex.y - currentBitmap.height) / atlas.height;
      //vertex 2
      descriptor->textureCoordinate[4] = (topLeftVertex.x + currentBitmap.width) / atlas.width;
      descriptor->textureCoordinate[5] = topLeftVertex.y / atlas.height;
      //vertex 3
      descriptor->textureCoordinate[6] = topLeftVertex.x / atlas.width;
      descriptor->textureCoordinate[7] = topLeftVertex.y / atlas.height;
    }

    //set character descriptor values
    descriptor->width = currentBitmap.width;
    descriptor->height = currentBitmap.height;

    //advance the pixel pointer
    pixelPointer += currentBitmap.width + PADDING; //advance the pixel pointer by the width of the drawn bitmap

    //recaculate the tallest bitmapo agaisnt drawn bitmap
    if(tallestBitmap < currentBitmap.height + PADDING)
    {
      tallestBitmap = currentBitmap.height + PADDING;
    }
  }

  //NOTE(Noah): the descriptors are stored into the asset via a float array which will not support other data we wish to store into the descriptors
  //push atlas and character descriptor array into the asset
  PushBitmapToAsset(atlas, &asset, arena);
  PushStructArrayToAsset((void *)descriptors, sizeof(character_desriptor) * (127 - 33), &asset, arena);

  //TODO(Noah): clean up the temporary memory that was used in the arena
  //clean up memory and return
  fileResult.free(FreeFile);
  return asset;
}

int main()
{
  char win32FilePath[MAX_PATH];
  Win32GetRelativePath(win32FilePath, MAX_PATH);

  unsigned int storageSize = MB(64);
  void *storage = VirtualAlloc(0, storageSize, MEM_COMMIT, PAGE_READWRITE);

  memory_arena arena = {};
  arena.init(storage, storageSize);

  char fontName[MAX_PATH];
  read_file_result fileResult = Win32ReadFile(MaccisCatStringsUnchecked(win32FilePath, "config\\font.txt", fontName));
  if(fileResult.content)
  {
    CloneString((char *)fileResult.content, fontName, fileResult.contentSize);
  } else
  {
    //TODO(Noah): logging: we could not find the font configuration file!
  }

  //Im going to need the file to save to
  //and im going to need the font name!

  //build the font asset
  loaded_asset asset = BuildFontAsset2(Win32ReadFile, Win32FreeFile, Win32WriteFile,
    &arena, fontName, 60.0f);

  //TODO(Noah): don't save to font.asset since that assumes that we are only going to have one font being used during runtime!
  WriteAsset(Win32WriteFile, &arena, &asset, MaccisCatStringsUnchecked(win32FilePath, "res\\font.asset", fontName));

  //output debug bitmap so we can see what the bitmap actually looks like to make sure the font is like ok fam!
  loaded_bitmap *bitmap = (loaded_bitmap *)asset.pWrapper->asset;
  SaveBitmap(MaccisCatStringsUnchecked(win32FilePath, "res\\fontAtlas.bmp", fontName), Win32WriteFile, *bitmap, &arena);
  VirtualFree(storage, storageSize, MEM_RELEASE);
}
