struct vertex_buffer
{
  unsigned int id;
  unsigned int size;
  unsigned int elementSize;
  void bind()
  {
    glBindBuffer(GL_ARRAY_BUFFER, id);
  }
};

struct buffer_element
{
  unsigned int componentCount;
  unsigned int componentSize;
  GLenum type;
  GLenum normalized;
};

struct buffer_layout
{
  buffer_element elements[10];
  unsigned int elementCount;
  unsigned int stride;
  void push(unsigned int count, GLenum type)
  {
    buffer_element element;
    element.componentCount = count;
    switch (type)
    {
      case GL_FLOAT:
      {
        element.type = GL_FLOAT;
        element.normalized = GL_FALSE;
        element.componentSize = sizeof(float);
      } break;
      default:
      {
        element.type = GL_FLOAT;
        element.normalized = GL_FALSE;
        element.componentSize = sizeof(float);
      } break;
    }
    if(elementCount < 10) {
      elements[elementCount++] = element;
      stride += element.componentCount * element.componentSize;
    }
  }
};

struct index_buffer
{
  unsigned int id;
  unsigned int count;
  void bind()
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
  }
};

struct vertex_array
{
  unsigned int id;
  vertex_buffer vertexBuffer;
  void bind()
  {
    glBindVertexArray(id);
  }
  void addBuffer(vertex_buffer vb, buffer_layout bl)
  {
    vertexBuffer = vb;
    vb.bind();
    bind();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < bl.elementCount; i++)
    {
        buffer_element element = bl.elements[i];
        glVertexAttribPointer(i, element.componentCount, element.type, element.normalized, bl.stride, (const void *)offset);
        glEnableVertexAttribArray(i);
        offset += element.componentSize * element.componentCount;
    }
  }
};

struct shader
{
  unsigned int id;
  void bind()
  {
    glUseProgram(id);
  }
  int getUnfiformLocation(char *name)
  {
    return glGetUniformLocation(id, name);
  }
  void setUniform4f(char *name, float x, float y, float z, float w)
  {
    glUniform4f(getUnfiformLocation(name), x, y, z, w);
  }
  void setUniform1i(char *name, int i)
  {
    glUniform1i(getUnfiformLocation(name), i);
  }
};

struct material_element
{
  vec4 element;
  bool set;
  int location;
};

struct material
{
  shader sh;
  vec4 color;
  int texture;
  void setColor(float r, float g, float b, float a)
  {
    color.x = r;
    color.y = g;
    color.z = b;
    color.w = a;
  }
  void setTexture(unsigned int slot)
  {
    texture = slot;
  }
  void updateUniforms()
  {
    sh.setUniform4f("ucolor", color.x, color.y, color.z, color.w);
    sh.setUniform1i("utexture", texture);
  }
};

struct texture
{
  unsigned int id;
  loaded_bitmap localTexture;
  void bind(unsigned int slot)
  {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
  }
  void del()
  {
    glDeleteTextures(1, &id);
  }
};
