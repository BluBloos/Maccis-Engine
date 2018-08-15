/* dependencies
maccis_math.h
backend.h
*/

inline void BindVertexBuffer(vertex_buffer *vertexBuffer)
{
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id);
}

inline void UnbindVertexBuffer()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline void DeleteVertexBuffer(vertex_buffer *vertexBuffer)
{
  glDeleteBuffers(1, &vertexBuffer->id);
}

inline void BindIndexBuffer(index_buffer *indexBuffer)
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->id);
}

inline void UnbindIndexBuffer()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline void DeleteIndexBuffer(index_buffer *indexBuffer)
{
  glDeleteBuffers(1, &indexBuffer->id);
}

inline void PushToBufferLayout(buffer_layout *bufferLayout, unsigned int count, unsigned int type)
{
  buffer_layout_element element;
  element.componentCount = count;
  switch (type)
  {
    case MACCIS_FLOAT:
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
  if(bufferLayout->elementCount < 10) {
    bufferLayout->elements[bufferLayout->elementCount++] = element;
    bufferLayout->stride += element.componentCount * element.componentSize;
  }
}

inline void BindVertexArray(vertex_array *vertexArray)
{
  glBindVertexArray(vertexArray->id);
}

inline void UnbindVertexArray()
{
    glBindVertexArray(0);
}

inline void AddBufferToVertexArray(vertex_array *vertexArray, vertex_buffer *vertexBuffer, buffer_layout *bufferLayout)
{
  vertexArray->vertexBuffer = *vertexBuffer;
  vertexArray->bufferLayout = *bufferLayout;
  BindVertexBuffer(vertexBuffer);
  BindVertexArray(vertexArray);

  unsigned int offset = 0;
  for (unsigned int i = 0; i < bufferLayout->elementCount; i++)
  {
      buffer_layout_element element = bufferLayout->elements[i];
      glVertexAttribPointer(i, element.componentCount, element.type, element.normalized, bufferLayout->stride, (const void *)offset);
      glEnableVertexAttribArray(i);
      offset += element.componentSize * element.componentCount;
  }

  UnbindVertexBuffer();
  UnbindVertexArray();
}

inline void BindShader(shader *shader)
{
  glUseProgram(shader->id);
}

inline int GetUniformLocationFromShader(shader *shader, char *name)
{
  return glGetUniformLocation(shader->id, name);
}

inline void SetUniform4f(shader *shader, char *name, float x, float y, float z, float w)
{
  glUniform4f(GetUniformLocationFromShader(shader, name), x, y, z, w);
}

inline void SetUniform1i(shader *shader, char *name, int i)
{
  glUniform1i(GetUniformLocationFromShader(shader, name), i);
}

inline void SetUniformMat4f(shader *shader, char *name, mat4 matrix)
{
  glUniformMatrix4fv(GetUniformLocationFromShader(shader, name), 1, GL_FALSE, &matrix.matp[0]);
}

inline void BindTexture(texture *texture)
{
  glActiveTexture(GL_TEXTURE0 + texture->slot);
  glBindTexture(GL_TEXTURE_2D, texture->id);
}

inline void DeleteTexture(texture *texture)
{
  glDeleteTextures(1, &texture->id);
}

inline vertex_array CreateVertexArray()
{
  vertex_array vertexArray = {};
  glGenVertexArrays(1, &vertexArray.id);
  return vertexArray;
}

inline buffer_layout CreateBufferLayout()
{
  buffer_layout bufferLayout = {};
  return bufferLayout;
}

inline index_buffer CreateIndexBuffer(unsigned int *data, unsigned int indexCount)
{
  index_buffer buffer = {}; buffer.count = indexCount;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.count * sizeof(unsigned int), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return buffer;
}

inline texture CreateTexture(unsigned int *pixelPointer, unsigned int width, unsigned int height, unsigned int slot)
{
  texture newTexture;
  glGenTextures(1, &newTexture.id);
  glBindTexture(GL_TEXTURE_2D, newTexture.id);

  newTexture.pixelPointer = pixelPointer;
  newTexture.width = width;
  newTexture.height = height;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelPointer);
  glBindTexture(GL_TEXTURE_2D, 0);

  newTexture.slot = slot;
  return newTexture;
}

vertex_buffer CreateVertexBuffer(float *data, unsigned int floatCount)
{
  vertex_buffer buffer = {}; buffer.elementSize = sizeof(float); buffer.size = buffer.elementSize * floatCount;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ARRAY_BUFFER, buffer.id); //select the buffer
  glBufferData(GL_ARRAY_BUFFER, buffer.size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return buffer;
}

//TODO(Noah): add logging to incorrect compilation of shaders
inline INTERNAL unsigned int CompileShader(unsigned int type, char *shader)
{
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &shader, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    #if 0
    //TODO(Noah): remove _alloca and add logging functions provided by the platform layer
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)_alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    printf("Failed to comile shader!\n");
    printf("%s\n", message);
    #endif
    glDeleteShader(id);
    return 0;
  }

  return id;
}

inline INTERNAL shader CreateShader(char *vertexShader, char *fragmentShader)
{
  shader s;
  unsigned int program = glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program, vs);
  glAttachShader(program, fs);

  //TODO(Noah): Assert if the shader compilation fails

  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  s.id = program;
  return s;
}
