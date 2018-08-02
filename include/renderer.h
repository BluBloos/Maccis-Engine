struct vertex_buffer
{
  unsigned int id;
  unsigned int size;
  unsigned int elementSize;
  void bind()
  {
    glBindBuffer(GL_ARRAY_BUFFER, id);
  }
  void unbind()
  {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  buffer_layout bufferLayout;
  void bind()
  {
    glBindVertexArray(id);
    for (unsigned int i = 0; i < bufferLayout.elementCount; i++)
    {
      glEnableVertexAttribArray(i);
    }
  }
  void unbind()
  {
    glBindVertexArray(0);
  }
  void addBuffer(vertex_buffer vb, buffer_layout bl)
  {
    vertexBuffer = vb; bufferLayout = bl;
    vb.bind();
    bind();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < bl.elementCount; i++)
    {
        buffer_element element = bl.elements[i];
        glVertexAttribPointer(i, element.componentCount, element.type, element.normalized, bl.stride, (const void *)offset);
        //glEnableVertexAttribArray(i);
        offset += element.componentSize * element.componentCount;
    }
    vb.unbind();
    unbind();
  }
};

struct shader
{
  unsigned int id;
  void bind()
  {
    glUseProgram(id);
  }
  int getUniformLocation(char *name)
  {
    return glGetUniformLocation(id, name);
  }
  void setUniform4f(char *name, float x, float y, float z, float w)
  {
    glUniform4f(getUniformLocation(name), x, y, z, w);
  }
  void setUniform1i(char *name, int i)
  {
    glUniform1i(getUniformLocation(name), i);
  }
  void setUniformMat4f(char *name, mat4 mat)
  {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat.matp[0]);
  }
};

struct transform
{
  mat4 matrix;
  vec3 position;
  vec3 scl;
  vec3 rotation;
  void bindMatrix(shader sh)
  {
    float array[] = {
      scl.x,0,0,0,
      0,scl.y,0,0,
      0,0,scl.z,0,
      0,0,0,1
    };

    mat4 bufferMatrix = {};
    for (unsigned int x = 0; x < 16; x++)
    {
      bufferMatrix.matp[x] = array[x];
    }

    float array2[] = {
      cosf(rotation.y * DEGREES_TO_RADIANS), 0, sinf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 1, 0, 0,
      -sinf(rotation.y * DEGREES_TO_RADIANS), 0, cosf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixY = {};
    for (unsigned int x = 0; x < 16; x++)
    {
      rotationMatrixY.matp[x] = array2[x];
    }

    bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixY);
    bufferMatrix.mat[3][0] = position.x;
    bufferMatrix.mat[3][1] = position.y;
    bufferMatrix.mat[3][2] = position.z;

    sh.setUniformMat4f("umodel", bufferMatrix);
  }
  void translate(float dx, float dy, float dz)
  {
    position.x += dx; position.y += dy; position.z += dz;
  }
  void setPosition(float x, float y, float z)
  {
    position.x = x; position.y = y; position.z = z;
  }
  void scale(float x, float y, float z)
  {
    scl.x *= x; scl.y *= y; scl.z *= z;
  }
  void setScale(float x, float y, float z)
  {
    scl.x = x; scl.y = y; scl.z = z;
  }
  void setRotation(float x, float y, float z)
  {
    rotation.x = x; rotation.y = y; rotation.z = z;
  }
  void rotate(float dx, float dy, float dz)
  {
    rotation.x += dx; rotation.y += dy; rotation.z += dz;
  }
};

struct camera
{
  mat4 view;
  mat4 proj;
  transform trans;
  void updateMatrix()
  {
    //update the view matrix
  }
  void bind(shader sh)
  {
    sh.setUniformMat4f("uproj", proj);
    //sh.setUniformMat4f("uview", view);
  }
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

struct mesh
{
  vertex_array vao;
  index_buffer indexBuffer;
  void bind()
  {
    vao.bind();
    indexBuffer.bind();
  }
};

struct game_object
{
  mesh mesh;
  transform transform;
  material material;
  void bind()
  {
    material.sh.bind();
    material.updateUniforms();
    transform.bindMatrix(material.sh);
    mesh.bind();
  }
};
