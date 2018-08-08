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
