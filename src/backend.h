struct index_buffer
{
  unsigned int id;
  unsigned int count;
};

struct vertex_buffer
{
  unsigned int id;
  unsigned int size;
  unsigned int elementSize;
};

struct buffer_layout_element
{
  unsigned int componentCount;
  unsigned int componentSize;
  unsigned int type;
  unsigned int normalized;
};

struct buffer_layout
{
  buffer_layout_element elements[10];
  unsigned int elementCount;
  unsigned int stride;
};

struct vertex_array
{
  unsigned int id;
  vertex_buffer vertexBuffer;
  buffer_layout bufferLayout;
};

struct shader
{
  unsigned int id;
};

struct texture
{
  unsigned int id;
  unsigned int slot;
  unsigned int width;
  unsigned int height;
  unsigned int *pixelPointer;
};
