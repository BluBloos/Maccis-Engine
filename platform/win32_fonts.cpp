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

#if 0
INTERNAL loaded_asset BuildFontAsset(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
  memory_arena *arena, char * font, float pixelHeight)
{
  loaded_asset asset = {};

  char stringBuffer[MAX_PATH];
  MaccisCatStringsUnchecked("C:\\Windows\\Fonts\\", font, stringBuffer);
  read_file_result fileResult = ReadFile(stringBuffer);

  for (unsigned int i = 33; i < 127; i++)
  {
    //TODO(Implement freeing on the memory_arena so that we can destroy the unused bitmaps!)
    loaded_bitmap bitmap = BuildCharacterBitmap(fileResult, i, pixelHeight, arena);
    PushBitmapToAsset(bitmap, &asset, arena);
  }

  fileResult.free(FreeFile);
  return asset;
}
#else
INTERNAL loaded_asset BuildFontAsset(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
  memory_arena *arena, char * font, float pixelHeight)
{
  loaded_asset asset = {};

  char stringBuffer[MAX_PATH];
  MaccisCatStringsUnchecked("C:\\Windows\\Fonts\\", font, stringBuffer);
  read_file_result fileResult = ReadFile(stringBuffer);

  loaded_bitmap characters[127 - 33] = {};
  unsigned int index = 0;

  for (unsigned int i = 33; i < 127; i++)
  {
    index = i - 33;
    //TODO(Implement freeing on the memory_arena so that we can destroy the unused bitmaps!)
    characters[index] = BuildCharacterBitmap(fileResult, i, pixelHeight, arena);
  }

  unsigned int pixelPitch = 1080;
  unsigned int tallestBitmap = 0;
  unsigned int currentLine = 0;
  unsigned int currentX = 0;
  unsigned int totalHeight = 0;
  for(unsigned int i = 0; i < 127 - 33; i++)
  {
    currentX += characters[i].width;
    if (currentX > pixelPitch)
    {
      currentX = characters[i].width; //advance the x of the new line to be the bitmap wrapped down to it
      currentLine += 1; //go to next line
      totalHeight += tallestBitmap; //add the height of the last line to the total height
      tallestBitmap = characters[i].height; //set the talles bitmap to the height of the wrapped bitmap
    }
    if(tallestBitmap < characters[i].height)
    {
      tallestBitmap = characters[i].height;
    }
  }

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
    currentX += currentBitmap.width;

    if (currentX > pixelPitch)
    {
      pixelPointer += pixelPitch - (currentX - currentBitmap.width); //advance the pixel pointer to the start of the next line
      pixelPointer += pixelPitch * (tallestBitmap - 1); //advance the pixel pointer by tallestBitmap - 1 lines, which advances us to the next "line"
      DrawBitmapUnchecked(currentBitmap, pixelPointer, atlas.width);

      currentX = currentBitmap.width; //advance the x of the new line to be the bitmap wrapped down to it
      currentLine += 1; //go to next lines
      totalHeight += tallestBitmap; //add the height of the last line to the total height
      tallestBitmap = currentBitmap.height; //set the talles bitmap to the height of the wrapped bitmap

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

      vec2 topLeftVertex = NewVec2((float)currentX - currentBitmap.width, (float)totalHeight + currentBitmap.height);
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
    pixelPointer += currentBitmap.width; //advance the pixel pointer by the width of the drawn bitmap

    //recaculate the tallest bitmapo agaisnt drawn bitmap
    if(tallestBitmap < currentBitmap.height)
    {
      tallestBitmap = currentBitmap.height;
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
#endif
