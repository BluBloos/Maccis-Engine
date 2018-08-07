#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

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

INTERNAL loaded_asset BuildFontAsset(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
  memory_arena *arena, char * font, float pixelHeight)
{
  loaded_asset asset = {};

  char stringBuffer[MAX_PATH];
  MaccisCatStringsUnchecked("C:\\Windows\\Fonts\\", font, stringBuffer);
  read_file_result fileResult = ReadFile(stringBuffer);

  for (unsigned int i = 32; i < 127; i++)
  {
    //TODO(Implement freeing on the memory_arena so that we can destroy the unused bitmaps!)
    loaded_bitmap bitmap = BuildCharacterBitmap(fileResult, i, pixelHeight, arena);
    PushBitmapToAsset(bitmap, &asset, arena);
  }

  fileResult.free(FreeFile);
  return asset;
}
