INTERNAL void GLClearError()
{
  while(glGetError() != GL_NO_ERROR);
}

INTERNAL bool GLCheckError(char *function, char *file, int line)
{
  bool result = true;
  while(GLenum error = glGetError())
  {
    printf("GL_ERROR: %d\nFUNCTION: %s\nFILE: %s\nLINE: %d", error, function, file, line);
    result = false;
  }
  return result;
}

#ifdef DEBUG
#define GL_CALL(code) GLClearError(); code; Assert(GLCheckError(#code, __FILE__, __LINE__));
#else
#define GL_CALL(code) code;
#endif

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
  void del()
  {
    glDeleteBuffers(1, &id);
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
  void del()
  {
    glDeleteBuffers(1, &id);
  }
  void unbind()
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
  vec3 position;
  vec3 scl;
  vec3 rotation;

  //NOTE(Noah): These are the three local basic vectors representing orientation for use with local translations!
  vec3 right;
  vec3 up;
  vec3 forward;

  mat4 buildMatrixOriginRotation()
  {
    float array[] = {
      scl.x,0,0,0,
      0,scl.y,0,0,
      0,0,scl.z,0,
      0,0,0,1
    };

    mat4 bufferMatrix = {};
    memcpy(bufferMatrix.matp, array, 16 * sizeof(float));

    bufferMatrix.mat[3][0] = position.x;
    bufferMatrix.mat[3][1] = position.y;
    bufferMatrix.mat[3][2] = position.z;

    float array2[] = {
      cosf(rotation.y * DEGREES_TO_RADIANS), 0, sinf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 1, 0, 0,
      -sinf(rotation.y * DEGREES_TO_RADIANS), 0, cosf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixY = {};
    memcpy(rotationMatrixY.matp, array2, 16 * sizeof(float));

    bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixY);

    float array3[] = {
      1, 0, 0, 0,
      0, cosf(rotation.x * DEGREES_TO_RADIANS), sinf(rotation.x * DEGREES_TO_RADIANS), 0,
      0, -sinf(rotation.x * DEGREES_TO_RADIANS), cosf(rotation.x * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixZ = {};
    memcpy(rotationMatrixZ.matp, array3, 16 * sizeof(float));

    bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixZ);

    return bufferMatrix;
  }
  mat4 buildMatrix()
  {
    float array[] = {
      scl.x,0,0,0,
      0,scl.y,0,0,
      0,0,scl.z,0,
      0,0,0,1
    };

    mat4 bufferMatrix = {};
    memcpy(bufferMatrix.matp, array, 16 * sizeof(float));

    float array2[] = {
      cosf(rotation.y * DEGREES_TO_RADIANS), 0, sinf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 1, 0, 0,
      -sinf(rotation.y * DEGREES_TO_RADIANS), 0, cosf(rotation.y * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixY = {};
    memcpy(rotationMatrixY.matp, array2, 16 * sizeof(float));

    bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixY);
    bufferMatrix.mat[3][0] = position.x;
    bufferMatrix.mat[3][1] = position.y;
    bufferMatrix.mat[3][2] = position.z;

    return bufferMatrix;
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
  //mat4 view;
  mat4 proj;
  transform trans;

  void bind(shader sh)
  {
    sh.setUniformMat4f("uproj", proj);
    sh.setUniformMat4f("uview", trans.buildMatrixOriginRotation());
  }

  void translateLocal(float dx, float dy, float dz)
  {
    trans.position = AddVec3(trans.position, ScaleVec3(trans.right, -dx) );
    trans.position = AddVec3(trans.position, ScaleVec3(trans.up, -dy) );
    trans.position = AddVec3(trans.position, ScaleVec3(trans.forward, -dz) );
  }

  void translate(float dx, float dy, float dz)
  {
    trans.position.x -= dx; trans.position.y -= dy; trans.position.z -= dz;
  }

  void setPosition(float x, float y, float z)
  {
    trans.position.x = -x; trans.position.y = -y; trans.position.z = -z;
  }

  void setRotation(float x, float y, float z)
  {
    trans.rotation.x = -x; trans.rotation.y = -y; trans.rotation.z = -z;
    //calculate basic vectors based on the rotation
  }

  void rotate(float dx, float dy, float dz)
  {
    trans.rotation.x -= dx; trans.rotation.y -= dy; trans.rotation.z -= dz;

    //create the rotation matrix
    float array[] = {
      cosf(-trans.rotation.y * DEGREES_TO_RADIANS), 0, sinf(-trans.rotation.y * DEGREES_TO_RADIANS), 0,
      0, 1, 0, 0,
      -sinf(-trans.rotation.y * DEGREES_TO_RADIANS), 0, cosf(-trans.rotation.y * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixY = {};
    memcpy(rotationMatrixY.matp, array, 16 * sizeof(float));

    trans.forward = TransformVec3Mat4(NewVec3(0.0f, 0.0f, 1.0f), rotationMatrixY);
    trans.right = TransformVec3Mat4(NewVec3(1.0f, 0.0f, 0.0f), rotationMatrixY);

  }
};

struct texture
{
  unsigned int id;
  unsigned int slot;
  loaded_bitmap localTexture;
  void bind()
  {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
  }
  void del()
  {
    glDeleteTextures(1, &id);
  }
};

struct material
{
  shader sh;
  vec4 color;
  texture tex;
  void setColor(float r, float g, float b, float a)
  {
    color.x = r;
    color.y = g;
    color.z = b;
    color.w = a;
  }
  void setTexture(texture t)
  {
    tex = t;
  }
  void updateUniforms()
  {
    sh.setUniform4f("ucolor", color.x, color.y, color.z, color.w);
    sh.setUniform1i("utexture", tex.slot);
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
    material.tex.bind();
    material.updateUniforms();
    material.sh.setUniformMat4f("umodel", transform.buildMatrix());
    mesh.bind();
  }
};

struct renderable_2D_vertex
{
  float position[2];
  float textureCoordinate[2];
  float normal[3];
};

//NOTE()
struct renderable_2D
{
  vec2 scale;
  vec2 position;
  float rotation;
  texture tex;
  renderable_2D_vertex vertices[4];
};

struct batch_renderer_2D
{
  vertex_array vao;
  index_buffer indexBuffer;
  renderable_2D_vertex *vertexBufferMap;
  shader defaultShader;
  texture textureAtlas;
};

struct engine_memory
{
  void *storage;
  long int storageSize;
  char *maccisDirectory;
  platform_read_file *ReadFile;
  platform_free_file *FreeFile;
  platform_get_clock *StartClock;
  platform_get_clock *EndClock;
  platform_get_delta_time *GetClockDeltaTime;
};

struct engine_state
{
  float elapsedTime;
  char character;
  //TODO(Noah): need the ability to free allocated memory on memoryArena
  memory_arena memoryArena;
  texture defaultTexture;
  texture fontAtlas;
  game_object defaultObject;
  game_object suzanne;
  camera mainCamera;
  camera guiCamera;
  loaded_bitmap font[100];
  loaded_asset fontAsset;
  loaded_asset monkeyAsset;
  //NOTE(Noah): unless otherwise specified by the game the batch renderer is not initialized
  //thats why it's a pointer
  batch_renderer_2D *batchRenderer2D;
  //TODO(Noah): Change this number from 90
  renderable_2D fontSprites[100];
};
