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
  memory_arena *arena, character_desriptor* descriptorOut, loaded_font *outFont)
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
#define MAX_WIDTH 1024
#define MAX_HEIGHT 1024
INTERNAL loaded_bitmap BuildCharacterBitmap(FILE *file, win32_font_context *fontContext, unsigned int character,
   memory_arena *arena, character_desriptor* descriptorOut, unsigned int firstChar)
{
  loaded_bitmap result = {};

  SIZE size = {};
  wchar_t cheesePoint = (wchar_t)character;

  GetTextExtentPoint32W(fontContext->dc, &cheesePoint, 1, &size);

  int width = size.cx;
  int height = size.cy;

  SetTextColor(fontContext->dc, RGB(255, 255, 255));
  SetBkColor(fontContext->dc, RGB(0,0,0));
  TextOutW(fontContext->dc, 0, 0, &cheesePoint, 1);

  int minX = 10000;
  int maxX = -10000;
  int minY = 10000;
  int maxY = -10000;

  unsigned int *row = (unsigned int *)fontContext->bPixels + MAX_WIDTH * (MAX_HEIGHT - 1);
  for (int y = 0; y < height; y++)
  {
    unsigned int *pixel = row;
    for (int x = 0; x < width; x++)
    {
      //COLORREF pixel = GetPixel(deviceContext, x, y);

      if (*pixel++ & 0xFF) //there is pixel!
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
    row -= MAX_WIDTH;
  }

  if (minX != 10000) //did we find anything?
  {
    width = maxX - minX + 1;
    height = maxY - minY + 1;

    //log the letter widths
    {
      fprintf(file,"letter: %d, width: %d\n", character, width);
    }

    descriptorOut->alignPercentage[0] = (float)minX / (float)width;
    descriptorOut->alignPercentage[1] = ((float)fontContext->metrics.tmDescent - (size.cy - maxY)) / (float)height;

    //Allocate a bitmap so we can fill it!
    result = MakeEmptyBitmap(arena, width, height);

    unsigned int *destRow = result.pixelPointer + (height - 1) * width;
    unsigned int *sourceRow = (unsigned int *)fontContext->bPixels + (MAX_HEIGHT - 1 - minY) * MAX_WIDTH;
    for (unsigned int y = minY; y < maxY + 1; y++)
    {
      unsigned int *source = sourceRow + minX;
      unsigned int *dest = destRow;
      for (unsigned int x = minX; x < maxX + 1; x++)
      {
        //COLORREF pixel = GetPixel(deviceContext, x, y);
        unsigned char alpha =  (unsigned char)(*source++ & 0xFF);
        *dest++ = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
      }
      sourceRow -= MAX_WIDTH;
      destRow -= width;
    }
  }

  //finally return the bitmap which was so painstakingly crafted!
  return result;
}
#endif

INTERNAL void BuildFontMeta(char *path, char *fontName, float pixelHeight,
  win32_font_context *fontContext, loaded_font *outFont, memory_arena *arena)
{
  AddFontResourceEx(path, FR_PRIVATE, 0);

  fontContext->font = CreateFontA((int)pixelHeight, 0, 0, 0,
    FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE, fontName);

  fontContext->dc = CreateCompatibleDC(NULL);

  BITMAPINFO info = {};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = MAX_WIDTH;
  info.bmiHeader.biHeight = MAX_HEIGHT;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  fontContext->bitmap = CreateDIBSection(fontContext->dc, &info, DIB_RGB_COLORS, (void **)&fontContext->bPixels, 0, 0);

  SelectObject(fontContext->dc, fontContext->bitmap);
  SelectObject(fontContext->dc, fontContext->font);

  GetTextMetrics(fontContext->dc, &fontContext->metrics);
  outFont->lineHeight = fontContext->metrics.tmHeight;
  outFont->firstChar = Max((float)fontContext->metrics.tmFirstChar, 33);
  outFont->lastChar = Min((float)fontContext->metrics.tmLastChar, 254);
  outFont->codePointCount = outFont->lastChar;
  outFont->horizontalAdvance = (float *)arena->push(sizeof(float) * outFont->codePointCount * outFont->codePointCount);

  //Initialize kerning table
  fontContext->abc = (ABC *)arena->push(sizeof(ABC) * (outFont->codePointCount));
  GetCharABCWidthsW(fontContext->dc, 0, outFont->codePointCount - 1, fontContext->abc);
  for (unsigned int i = 0; i < outFont->codePointCount; i++)
  {
    ABC currentABC = fontContext->abc[i];
    float w = (float)currentABC.abcA + (float)currentABC.abcB + (float)currentABC.abcC;
    for (unsigned int j = 0; j < outFont->codePointCount; j++)
    {
      outFont->horizontalAdvance[i * outFont->codePointCount + j] = w;
    }
  }

  //log out the abc's!
  {
    FILE *file = fopen("C:\\dev\\abc.txt","w+");
    for (unsigned int i = 0; i < outFont->codePointCount; i++)
    {
      ABC currentABC = fontContext->abc[i];
      fprintf(file,"letter: %d, a: %d, b: %d, c: %d\n", i, currentABC.abcA, currentABC.abcB, currentABC.abcC);
    }
    fclose(file);
  }

  //add kerning values to kerning table
  DWORD kerningPairCount = GetKerningPairsW(fontContext->dc, 0, 0);
  fontContext->kerningPairs = (KERNINGPAIR *)arena->push(kerningPairCount * sizeof(KERNINGPAIR));
  GetKerningPairsW(fontContext->dc, kerningPairCount, fontContext->kerningPairs);
  for (DWORD kerningIndex = 0; kerningIndex < kerningPairCount; kerningIndex++ )
  {
    //NOTE(Noah): watch the offset of 33 there bud
    KERNINGPAIR pair = fontContext->kerningPairs[kerningIndex];
    if(pair.wFirst < outFont->codePointCount && pair.wSecond < outFont->codePointCount)
    {
      outFont->horizontalAdvance[pair.wFirst * outFont->codePointCount + pair.wSecond] += (float)pair.iKernAmount;
    }
  }

  //log out the kerning pairs!
  {
    FILE *file = fopen("C:\\dev\\kern.txt","w+");
    for (DWORD kerningIndex = 0; kerningIndex < kerningPairCount; kerningIndex++ )
    {
      KERNINGPAIR pair = fontContext->kerningPairs[kerningIndex];
      if(pair.wFirst < outFont->codePointCount && pair.wSecond < outFont->codePointCount)
      {
        fprintf(file, "letter 1: %d, letter 2: %d, kern: %d\n", pair.wFirst, pair.wSecond, pair.iKernAmount);
      }
    }
    fclose(file);
  }
}

#define PADDING 8
INTERNAL loaded_asset BuildFontAsset(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
  memory_arena *arena, char * font, float pixelHeight)
{
  loaded_asset asset = {};
  loaded_font outFont = {};

  char stringBuffer[MAX_PATH];
  char stringBuffer2[MAX_PATH];
  MaccisCatStringsUnchecked("C:\\Windows\\Fonts\\", font, stringBuffer);
  MaccisCatStringsUnchecked(stringBuffer, ".ttf", stringBuffer2);
  read_file_result fileResult = ReadFile(stringBuffer2);

  win32_font_context context = {};
  BuildFontMeta(stringBuffer2, font, pixelHeight,
      &context, &outFont, arena);

  loaded_bitmap *characters = (loaded_bitmap *)arena->push(sizeof(loaded_bitmap) * outFont.codePointCount);
  character_desriptor *descriptors = (character_desriptor *)arena->push(sizeof(character_desriptor) * outFont.codePointCount);
  unsigned int index = 0;

  FILE *file2 = fopen("C:\\dev\\widths.txt","w+");
  for (unsigned int i = outFont.firstChar; i < outFont.lastChar + 1; i++)
  {
    //TODO(Implement freeing on the memory_arena so that we can destroy the unused bitmaps!)
    #if DONT_USE_WINDOWS_FONTS
    characters[i] = BuildCharacterBitmap(fileResult, i, pixelHeight, arena, &descriptors[index], &outFont);
    #else
    characters[ii] = BuildCharacterBitmap(file2, &context, i, arena, &descriptors[index], outFont.firstChar);
    #endif
  }
  fclose(file2);
  /*
  //DEBUG save all the bitmaps to a file
  for (unsigned int i = 0; i < 127 -33; i++)
  {
    char stringBuffer[MAX_PATH];
    char stringBuffer2[MAX_PATH];
    wsprintf(stringBuffer, "res\\%d.bmp", i);
    SaveBitmap(MaccisCatStringsUnchecked("C:\\dev\\Maccis-Engine\\", stringBuffer, stringBuffer2), WriteFile, characters[i], arena);
  }
  */

  //do a pseudo run
  unsigned int pixelPitch = 1080;
  unsigned int atlasHeight = 0;
  unsigned int layerHeight = 0;
  unsigned int currentX = 0;
  for (unsigned int i = outFont.firstChar; i < outFont.lastChar + 1; i++)
  {
    loaded_bitmap currentBitmap = characters[i];
    if (currentX + currentBitmap.width + PADDING < pixelPitch)
    {
      //draw bitmap
      currentX += currentBitmap.width + PADDING; //advance
      if (currentBitmap.height + PADDING > layerHeight) //did the layer height increase?
      {
        layerHeight = currentBitmap.height + PADDING;
      }
    } else
    {
      //advance to next line
      atlasHeight += layerHeight;
      currentX = 0;
      layerHeight = 0;

      //query again
      if (currentX + currentBitmap.width + PADDING < pixelPitch)
      {
        //draw bitmap
        currentX += currentBitmap.width + PADDING; //advance
        if (currentBitmap.height + PADDING > layerHeight) //did the layer height increase?
        {
          layerHeight = currentBitmap.height + PADDING;
        }
      }
    }
  }

  atlasHeight += layerHeight;
  void *mem = arena->push(sizeof(unsigned int) * pixelPitch * atlasHeight);

  loaded_bitmap atlas = {};
  atlas.pixelPointer = (unsigned int *)mem;
  atlas.width = pixelPitch;
  atlas.height = atlasHeight;

  pixelPitch = 1080;
  atlasHeight = 0;
  unsigned int *pixelPointer = atlas.pixelPointer;
  layerHeight = 0;
  currentX = 0;
  for (unsigned int i = outFont.firstChar; i < outFont.lastChar + 1; i++)
  {
    loaded_bitmap currentBitmap = characters[i];
    character_desriptor *descriptor = &descriptors[i];

    if (currentX + currentBitmap.width + PADDING < pixelPitch)
    {
      //draw bitmap
      DrawBitmapUnchecked(currentBitmap, pixelPointer, atlas.width);

      vec2 topLeftVertex = NewVec2((float)currentX, (float)atlasHeight + currentBitmap.height);
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

      currentX += currentBitmap.width + PADDING; //advance
      pixelPointer += currentBitmap.width + PADDING;
      if (currentBitmap.height + PADDING > layerHeight) //did the layer height increase?
      {
        layerHeight = currentBitmap.height + PADDING;
      }
    } else
    {
      //advance to next line
      pixelPointer += pixelPitch - currentX + (layerHeight - 1) * pixelPitch;
      atlasHeight += layerHeight;
      currentX = 0;
      layerHeight = 0;

      //query again
      if (currentX + currentBitmap.width + PADDING < pixelPitch)
      {
        //draw bitmap
        DrawBitmapUnchecked(currentBitmap, pixelPointer, atlas.width);

        vec2 topLeftVertex = NewVec2((float)currentX, (float)atlasHeight + currentBitmap.height);
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

        currentX += currentBitmap.width + PADDING; //advance
        pixelPointer += currentBitmap.width + PADDING;
        if (currentBitmap.height + PADDING > layerHeight) //did the layer height increase?
        {
          layerHeight = currentBitmap.height + PADDING;
        }
      }
    }

    descriptor->width = currentBitmap.width;
    descriptor->height = currentBitmap.height;

  }

  //NOTE(Noah): the descriptors are stored into the asset via a float array which will not support other data we wish to store into the descriptors
  //push atlas and character descriptor array into the asset
  PushBitmapToAsset(atlas, &asset, arena);
  PushStructArrayToAsset((void *)descriptors, sizeof(character_desriptor) * (outFont.codePointCount), &asset, arena);
  PushFontToAsset(&outFont, &asset, arena);

  FILE *file = fopen("C:\\dev\\log.txt","w+");
  for (unsigned int i = 0; i < outFont.codePointCount; i++)
  {
    for (unsigned int j = 0; j < outFont.codePointCount; j++)
    {
      float f = outFont.horizontalAdvance[i * outFont.codePointCount + j];
      fprintf(file, "letter 1: %d, letter 2: %d, f: %f\n",i + outFont.firstChar, j + outFont.firstChar, f);
    }
  }
  fclose(file);

  PushStructArrayToAsset((void *)outFont.horizontalAdvance, sizeof(float) * outFont.codePointCount * outFont.codePointCount, &asset, arena);

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
  loaded_asset asset = BuildFontAsset(Win32ReadFile, Win32FreeFile, Win32WriteFile,
    &arena, fontName, 60.0f);

  //TODO(Noah): don't save to font.asset since that assumes that we are only going to have one font being used during runtime!
  WriteAsset(Win32WriteFile, &arena, &asset, MaccisCatStringsUnchecked(win32FilePath, "res\\font.asset", fontName));

  //output debug bitmap so we can see what the bitmap actually looks like to make sure the font is like ok fam!
  loaded_bitmap *bitmap = (loaded_bitmap *)asset.pWrapper->asset;
  SaveBitmap(MaccisCatStringsUnchecked(win32FilePath, "res\\fontAtlas.bmp", fontName), Win32WriteFile, *bitmap, &arena);
  VirtualFree(storage, storageSize, MEM_RELEASE);
}
