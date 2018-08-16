/* dependencies
engine.h
string.h
*/

inline void Submit(batch_renderer_2D *batchRenderer2D, renderable_2D renderable)
{
  renderable.position = NewVec2(renderable.position.x + renderable.width * renderable.alignPercentage[0],
    renderable.position.y - renderable.height * renderable.alignPercentage[1]);

  renderable.vertices[0].position[0] += renderable.position.x;
  renderable.vertices[0].position[1] += renderable.position.y;

  renderable.vertices[1].position[0] += renderable.position.x;
  renderable.vertices[1].position[1] += renderable.position.y;

  renderable.vertices[2].position[0] += renderable.position.x;
  renderable.vertices[2].position[1] += renderable.position.y;

  renderable.vertices[3].position[0] += renderable.position.x;
  renderable.vertices[3].position[1] += renderable.position.y;

  memcpy(batchRenderer2D->vertexBufferMap, renderable.vertices,
    sizeof(renderable_2D_vertex) * 4);
  batchRenderer2D->vertexBufferMap += 4;
  batchRenderer2D->indexBuffer.count += 6;
}

//TODO(Noah): do these belong here?
unsigned int GetIndexFromCharacter(loaded_font *font, unsigned int character)
{
  return character - font->firstChar;
}

float GetHorizontalAdvanceForPair(loaded_font *font, unsigned char character1, unsigned char character2)
{
  //do stuff here
  float result = font->horizontalAdvance[character1 * font->codePointCount + character2];
  return result;
}

inline void DebugPushText(char *string, batch_renderer_2D *batchRenderer2D, loaded_font *font, vec2 position, unsigned int textAlignment)
{
  renderable_2D *fontSprites = (renderable_2D *)font->fontSprites;
  unsigned int stringLength = GetStringLength(string);

  float xOffset = position.x;
  float baseline = position.y;

  if (textAlignment == TEXT_CENTER)
  {
    unsigned int length = 0;
    char *scan = string;
    char prevCharacter = 0;
    for (unsigned int i = 0; i < stringLength; i++)
    {
      char character = *scan;

      if (prevCharacter != 0)
      {
        length += GetHorizontalAdvanceForPair(font, prevCharacter, character);
      }
      prevCharacter = *scan;
      scan++;
    }
    xOffset -= (float)length / 2.0f;
  }

  char *scan = string;
  char prevCharacter = 0;
  for (unsigned int i = 0; i < stringLength; i++)
  {
    char character = *scan;

    if (prevCharacter != 0)
    {
      xOffset += GetHorizontalAdvanceForPair(font, prevCharacter, character);
    }

    renderable_2D sprite = fontSprites[character];
    //sprite.position = NewVec2(xOffset + sprite.width * sprite.alignPercentage[0], baseline - sprite.height * sprite.alignPercentage[1]);
    sprite.position = NewVec2(xOffset, baseline);
    Submit(batchRenderer2D, sprite);
      //xOffset += sprite.width + 1.0f;

    prevCharacter = *scan;
    scan++;
  }

  //special case space
  //do proper verical alignment to the baseline via getting the text metrics
}
