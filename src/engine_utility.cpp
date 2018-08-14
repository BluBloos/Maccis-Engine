/* dependency
gl.h
engine.h
maccis_file_io.h
file_io.cpp
*/

vertex_array CreateVertexArray()
{
  vertex_array vertexArray = {};
  glGenVertexArrays(1, &vertexArray.id);
  return vertexArray;
}

buffer_layout CreateBufferLayout()
{
  buffer_layout bufferLayout = {};
  return bufferLayout;
}

index_buffer CreateIndexBuffer(unsigned int *data, unsigned int indexCount)
{
  index_buffer buffer = {}; buffer.count = indexCount;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.count * sizeof(unsigned int), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return buffer;
}

//TODO(Noah): fix this? or remove it. Why is it an engine engine utility.
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

engine_image BMPToImage(loaded_bitmap bitmap)
{
  engine_image image = {};
  image.scale = 1;
	image.pixelPointer = bitmap.pixelPointer;
	image.height = bitmap.height;
	image.width = bitmap.width;
	image.container = bitmap.container;
	return image;
}

//TODO(Noah): this function is temporary, its also a dumb wrapper
engine_image LoadBMPImage(platform_read_file *ReadFile, char *path)
{
	loaded_bitmap bitmap = LoadBMP(ReadFile, path);
	return BMPToImage(bitmap);
}
