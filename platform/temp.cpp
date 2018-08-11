#define PADDING 8

INTERNAL loaded_asset BuildFontAsset2(platform_read_file *ReadFile, platform_free_file *FreeFile, platform_write_file *WriteFile,
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
  for (unsigned int i = 0; i < 127 -33; i++)
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
  character_desriptor descriptors[127 - 33] = {};

  loaded_bitmap atlas = {};
  atlas.pixelPointer = (unsigned int *)mem;
  atlas.width = pixelPitch;
  atlas.height = atlasHeight;

  pixelPitch = 1080;
  atlasHeight = 0;
  unsigned int *pixelPointer = atlas.pixelPointer;
  layerHeight = 0;
  currentX = 0;
  for (unsigned int i = 0; i < 127 -33; i++)
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
  PushStructArrayToAsset((void *)descriptors, sizeof(character_desriptor) * (127 - 33), &asset, arena);

  //TODO(Noah): clean up the temporary memory that was used in the arena
  //clean up memory and return
  fileResult.free(FreeFile);
  return asset;
}
