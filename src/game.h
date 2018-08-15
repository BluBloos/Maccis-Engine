/*dependencies
maccis.h
maccis_file_io.h
maccis_asset.h
asset.cpp
maccis_math.h
engine.h
string.h
*/

INTERNAL mat4 CreateOrthographicMatrix(float width, float height, float n, float f)
{
  mat4 result = {};
  float r = width; float l = 0;
  float t = height; float b = 0;
  float identity[] = {
    2 / (r - l), 0, 0, 0,
    0, 2 / (t -b),  0, 0,
    0, 0, -2 / (f - n),0,
    -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n),1
  };
  memcpy(result.matp, identity, sizeof(float) * 16);
  return result;
}

INTERNAL camera CreateOrthoCamera(float width, float height)
{
  camera cam = {};
  cam.proj = CreateOrthographicMatrix(width, height, 1.0f, 100.0f);
  cam.trans.setScale(1.0f, 1.0f , 1.0f);
  cam.trans.forward = NewVec3(0.0f, 0.0f, 1.0f);
  cam.trans.up = NewVec3(0.0f, 1.0f, 0.0f);
  cam.trans.right = NewVec3(1.0f, 0.0f, 0.0f);
  return cam;
}

INTERNAL renderable_2D CreateSprite(float uniformScale, vec2 pos, float width, float height)
{
  renderable_2D renderable = {};

  //setup transform of renderable
  renderable.scale = NewVec2(uniformScale, uniformScale);

  //generate vertices of the renderable
  renderable.vertices[0].position[0] = pos.x;
  renderable.vertices[0].position[1] = pos.y;
  renderable.vertices[0].textureCoordinate[0] = 0.0f;
  renderable.vertices[0].textureCoordinate[1] = 0.0f;

  renderable.vertices[1].position[0] = pos.x + width * uniformScale;
  renderable.vertices[1].position[1] = pos.y;
  renderable.vertices[1].textureCoordinate[0] = 1.0f;
  renderable.vertices[1].textureCoordinate[1] = 0.0f;

  renderable.vertices[2].position[0] = pos.x + width * uniformScale;
  renderable.vertices[2].position[1] = pos.y + height * uniformScale;
  renderable.vertices[2].textureCoordinate[0] = 1.0f;
  renderable.vertices[2].textureCoordinate[1] = 1.0f;

  renderable.vertices[3].position[0] = pos.x;
  renderable.vertices[3].position[1] = pos.y + height * uniformScale;
  renderable.vertices[3].textureCoordinate[0] = 0.0f;
  renderable.vertices[3].textureCoordinate[1] = 1.0f;

  return renderable;
}

INTERNAL renderable_2D SpriteFromBitmap(loaded_bitmap bitmap)
{
  return CreateSprite(1.0f, NewVec2(0.0f, 0.0f), (float)bitmap.width, (float)bitmap.height);
}

INTERNAL renderable_2D SpriteFromTexture(float uniformScale, vec2 pos,
  texture tex)
{
  renderable_2D renderable = {};

  //setup transform of renderable
  renderable.scale = NewVec2(uniformScale, uniformScale);
  renderable.width = tex.width;
  renderable.height = tex.height;

  //generate vertices of the renderable
  renderable.vertices[0].position[0] = pos.x - tex.width * uniformScale / 2.0f;
  renderable.vertices[0].position[1] = pos.y - tex.height * uniformScale / 2.0f;
  renderable.vertices[0].textureCoordinate[0] = 0.0f;
  renderable.vertices[0].textureCoordinate[1] = 0.0f;

  renderable.vertices[1].position[0] = pos.x + tex.width * uniformScale / 2.0f;
  renderable.vertices[1].position[1] = pos.y - tex.height * uniformScale / 2.0f;
  renderable.vertices[1].textureCoordinate[0] = 1.0f;
  renderable.vertices[1].textureCoordinate[1] = 0.0f;

  renderable.vertices[2].position[0] = pos.x + tex.width * uniformScale / 2.0f;
  renderable.vertices[2].position[1] = pos.y + tex.height * uniformScale / 2.0f;
  renderable.vertices[2].textureCoordinate[0] = 1.0f;
  renderable.vertices[2].textureCoordinate[1] = 1.0f;

  renderable.vertices[3].position[0] = pos.x - tex.width * uniformScale / 2.0f;
  renderable.vertices[3].position[1] = pos.y + tex.height * uniformScale / 2.0f;
  renderable.vertices[3].textureCoordinate[0] = 0.0f;
  renderable.vertices[3].textureCoordinate[1] = 1.0f;

  return renderable;
}

void LoadFontFromAsset(memory_arena *arena, loaded_asset asset, loaded_font *outFont)
{
  asset_wrapper wrappers[4] = {};
  ParseAsset(&asset, wrappers);

  loaded_font *sourceFont = (loaded_font *)wrappers[2].asset;
  *outFont = *sourceFont;
  outFont->horizontalAdvance = (float  *)wrappers[3].asset;

  outFont->fontSprites = arena->push(outFont->codePointCount * sizeof(renderable_2D));
  renderable_2D *fontSprites = (renderable_2D *)outFont->fontSprites;

  character_desriptor *descriptors = (character_desriptor *)wrappers[1].asset;
  unsigned int descriptorCount = wrappers[1].assetSize / sizeof(character_desriptor);
  for (unsigned int i = 0; i < descriptorCount; i++)
  {
    character_desriptor descriptor = descriptors[i];
    fontSprites[i] = CreateSprite(1.0f, NewVec2(0.0f, 0.0f), descriptor.width, descriptor.height);
    fontSprites[i].width = descriptor.width;
    fontSprites[i].height = descriptor.height;
    fontSprites[i].alignPercentage[0] = descriptor.alignPercentage[0];
    fontSprites[i].alignPercentage[1] = descriptor.alignPercentage[1];
    fontSprites[i].vertices[0].textureCoordinate[0] = descriptor.textureCoordinate[0];
    fontSprites[i].vertices[0].textureCoordinate[1] = descriptor.textureCoordinate[1];
    fontSprites[i].vertices[1].textureCoordinate[0] = descriptor.textureCoordinate[2];
    fontSprites[i].vertices[1].textureCoordinate[1] = descriptor.textureCoordinate[3];
    fontSprites[i].vertices[2].textureCoordinate[0] = descriptor.textureCoordinate[4];
    fontSprites[i].vertices[2].textureCoordinate[1] = descriptor.textureCoordinate[5];
    fontSprites[i].vertices[3].textureCoordinate[0] = descriptor.textureCoordinate[6];
    fontSprites[i].vertices[3].textureCoordinate[1] = descriptor.textureCoordinate[7];
  }
}
