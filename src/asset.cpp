//returns the last wrapper in the asset
asset_wrapper *AdvanceToAssetEnd(loaded_asset *asset)
{
  asset_wrapper *pNext = asset->pWrapper;

  while(pNext)
  {
    if (pNext->pNext)
    {
      pNext = pNext->pNext;
    } else
    {
      break;
    }
  }

  return pNext;
}

asset_wrapper *AppendToAsset(memory_arena *arena, loaded_asset *asset, unsigned int assetSize)
{
  asset_wrapper *wrapper = (asset_wrapper *)arena->push(sizeof(asset_wrapper) + assetSize);
  wrapper->asset = wrapper + 1; //add one whole asset_wrapper
  asset->count += 1;

  asset_wrapper *lastWrapper = AdvanceToAssetEnd(asset);
  //asset_wrapper *bitmapWrapper = AppendToAsset(arena, asset, sizeof(loaded_bitmap));
  if (lastWrapper)
  {
    lastWrapper->pNext = wrapper;
  } else
  {
    asset->pWrapper = wrapper;
  }

  return wrapper;
}

//NOTE(Noah): the size is the size of the vetex and indices buffer!
void PushRawModelToAsset(raw_model model, loaded_asset *asset, memory_arena *arena)
{
  asset_wrapper *modelWrapper = AppendToAsset(arena, asset, sizeof(raw_model));
  memcpy(modelWrapper->asset, &model, sizeof(model));
  modelWrapper->assetType = ASSET_RAW_MODEL;
  //TODO(Noah): please remove the constant of 8 here, it scares me
  modelWrapper->assetSize = sizeof(raw_model) + model.vertexCount * sizeof(float) * 8 +
    model.indexCount * sizeof(unsigned int);
}

//NOTE(Noah): this will clone the bitmap
void PushBitmapToAsset(loaded_bitmap bitmap, loaded_asset *asset, memory_arena *arena)
{
  asset_wrapper *bitmapWrapper = AppendToAsset(arena, asset, sizeof(loaded_bitmap));
  memcpy(bitmapWrapper->asset, &bitmap, sizeof(loaded_bitmap));
  bitmapWrapper->assetType = ASSET_BITMAP;
  bitmapWrapper->assetSize = bitmap.width * bitmap.height * sizeof(unsigned int) + sizeof(loaded_bitmap);
}

void PushStructArrayToAsset(void *array, unsigned arraySize,
  loaded_asset *asset, memory_arena *arena)
{
  asset_wrapper *arrayWrapper = AppendToAsset(arena, asset, arraySize);
  memcpy(arrayWrapper->asset, array, arraySize);
  arrayWrapper->assetType = ASSET_STRUCT_ARRAY;
  arrayWrapper->assetSize = arraySize;
}

void PushFontToAsset(loaded_font *font, loaded_asset *asset, memory_arena *arena)
{
  asset_wrapper *fontWrapper = AppendToAsset(arena, asset, sizeof(loaded_font));
  memcpy(fontWrapper->asset, font, sizeof(loaded_font));
  fontWrapper->assetType = ASSET_FONT;
  fontWrapper->assetSize = sizeof(loaded_font);
}

void WriteAsset(platform_write_file *WriteFile, memory_arena *arena, loaded_asset *asset, char *fileName)
{
  unsigned int assetSize = asset->count * sizeof(unsigned int);

  asset_wrapper *pWrapper = asset->pWrapper;
  for (unsigned int i = 0; i < asset->count; i++)
  {
    assetSize += pWrapper->assetSize;
    pWrapper = pWrapper->pNext;
  }

  char *data = (char *)arena->push(assetSize);
  char *scan = data;

  //pack the data
  pWrapper = asset->pWrapper;
  for (unsigned int i = 0; i < asset->count; i++)
  {
    *scan = pWrapper->assetType;
    scan += sizeof(unsigned int);
    switch (pWrapper->assetType)
    {
      case ASSET_BITMAP:
      {
        memcpy(scan, pWrapper->asset, sizeof(loaded_bitmap));
        scan += sizeof(loaded_bitmap);
        loaded_bitmap bitmap = *(loaded_bitmap *)pWrapper->asset;
        memcpy(scan, bitmap.pixelPointer, pWrapper->assetSize - sizeof(loaded_bitmap));
        scan += pWrapper->assetSize - sizeof(loaded_bitmap);
      } break;
      case ASSET_RAW_MODEL:
      {
        memcpy(scan, pWrapper->asset, sizeof(raw_model));
        scan += sizeof(raw_model);
        raw_model model = *(raw_model *)pWrapper->asset;
        memcpy(scan, model.vertices, model.vertexCount * sizeof(float) * 8);
        scan += model.vertexCount * sizeof(float) * 8;
        memcpy(scan, model.indices, model.indexCount * sizeof(unsigned int));
        scan += model.indexCount * sizeof(unsigned int);
      } break;
      case ASSET_STRUCT_ARRAY:
      {
        *(unsigned int *)scan = pWrapper->assetSize;
        scan += sizeof(unsigned int);
        memcpy(scan, pWrapper->asset, pWrapper->assetSize);
        scan += pWrapper->assetSize;
      } break;
      case ASSET_FONT:
      {
        memcpy(scan, pWrapper->asset, sizeof(loaded_font));
        scan += sizeof(loaded_font);
      } break;
    }
    pWrapper = pWrapper->pNext;
  }

  WriteFile(fileName, assetSize, data);
}

loaded_asset LoadAsset(platform_read_file *ReadFile, platform_free_file *FreeFile,
  memory_arena *arena, char *path)
{
  loaded_asset asset = {};

  read_file_result fileResult = ReadFile(path);

  unsigned char *scan = (unsigned char *)fileResult.content;
  unsigned char *fileEnd = (unsigned char *)fileResult.content + fileResult.contentSize;
  while(scan < fileEnd)
  {
    unsigned int header = *(unsigned int *)scan;
    scan += sizeof(unsigned int);
    switch (header)
    {
      case ASSET_BITMAP:
      {
        loaded_bitmap bitmap = *(loaded_bitmap *)scan;
        scan += sizeof(loaded_bitmap);
        bitmap.pixelPointer = (unsigned int *)arena->push(bitmap.width * bitmap.height * sizeof(unsigned int));
        memcpy(bitmap.pixelPointer, scan, bitmap.width * bitmap.height * sizeof(unsigned int));
        PushBitmapToAsset(bitmap, &asset, arena);
        scan += bitmap.width * bitmap.height * sizeof(unsigned int);
      } break;
      case ASSET_RAW_MODEL:
      {
        raw_model *model = (raw_model *)scan;
        scan += sizeof(raw_model);
        unsigned int vertexSize = model->vertexCount * 8 * sizeof(float);
        unsigned int indexSize = model->indexCount * sizeof(unsigned int);
        model->vertices = arena->push(vertexSize);
        model->indices = arena->push(indexSize);
        memcpy(model->vertices, scan, vertexSize);
        scan += vertexSize;
        memcpy(model->indices, scan, indexSize);
        scan += indexSize;
        PushRawModelToAsset(*model, &asset, arena);
      } break;
      case ASSET_STRUCT_ARRAY:
      {
        unsigned int arraySize = *(unsigned int *)scan;
        scan += sizeof(unsigned int);
        PushStructArrayToAsset(scan, arraySize, &asset, arena);
        scan += arraySize;
      } break;
      case ASSET_FONT:
      {
        loaded_font *font = (loaded_font *)scan;
        scan += sizeof(loaded_font);
        PushFontToAsset(font, &asset, arena);
      } break;
    }
  }

  fileResult.free(FreeFile);
  return asset;
}

void ParseAsset(loaded_asset *asset, asset_wrapper *wrappers)
{
  asset_wrapper *pWrapper = asset->pWrapper;
  unsigned int index = 0;
  while(pWrapper)
  {
    wrappers[index++] = *pWrapper;
    if (pWrapper->pNext)
    {
      pWrapper = pWrapper->pNext;
    } else
    {
      break;
    }
  }
}

void ParseAssetOfBitmapList(loaded_asset *asset, loaded_bitmap *bitmaps)
{
  asset_wrapper *pWrapper = asset->pWrapper;

  unsigned int index = 0;
  while(pWrapper)
  {
    bitmaps[index++] = *(loaded_bitmap *)pWrapper->asset;
    if (pWrapper->pNext)
    {
      pWrapper = pWrapper->pNext;
    } else
    {
      break;
    }
  }
}
