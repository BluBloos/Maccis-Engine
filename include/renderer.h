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
    this->bind();
    for (unsigned int i = 0; i < bl.elementCount; i++)
    {
        buffer_element element = bl.elements[i];
        glVertexAttribPointer(i, element.componentCount, element.type, element.normalized, element.componentSize * element.componentCount, 0);
        glEnableVertexAttribArray(i);
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
  material_element color;
  void setColor(float r, float g, float b, float a)
  {
    color.element.x = r;
    color.element.y = g;
    color.element.z = b;
    color.element.w = a;
    if(!color.set){
      color.location = glGetUniformLocation(sh.id, "ucolor");
    }
    color.set = true;
  }
  void updateUniform()
  {
    if(color.set){
      glUniform4f(color.location, color.element.x, color.element.y, color.element.z, color.element.z);
    }
  }
};
