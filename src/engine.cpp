#include <maccis.h>
#include <maccis_math.h>
#include <renderer.cpp>
#include <engine.h>

INTERNAL float positions[] = {
  -0.5f, -0.5f, //0
   0.5f, -0.5f, //1
   0.5f,  0.5f, //2
  -0.5f,  0.5f  //3
};

INTERNAL unsigned int indices[] = {
  0, 1, 2,
  2, 3, 0
};

void CatStrings(int sourceACount, char *sourceA, int sourceBCount,
 char *sourceB, int destCount, char *dest)
{
	for (int index = 0; index < sourceACount; index++)
	{
		*dest++ = *sourceA++;
	}
	for (int index = 0; index < sourceBCount; index++)
	{
		*dest++ = *sourceB++;
	}
	*dest = 0;
}

unsigned int GetStringLength(char *string)
{
	unsigned int count = 0;
	while (*string != 0)
	{
		string++; count++;
	}
	return count;
}

char *BuildFilePath(file_path fpath, char *fileName, char *buffer, int bufferLength)
{
	CatStrings(fpath.length, fpath.path, GetStringLength(fileName), fileName, bufferLength, buffer);
  return buffer;
}

INTERNAL unsigned int CompileShader(unsigned int type, char *shader)
{
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &shader, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)_alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    //std::cout << "Failed to comile shader!" << std::endl;
    //std::cout << message << std::endl;
    glDeleteShader(id);
    return 0;
  }

  return id;
}

INTERNAL shader CreateShader(char *vertexShader, char *fragmentShader)
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

vertex_buffer CreateVertexBuffer(float *data, unsigned int count)
{
  vertex_buffer buffer; buffer.elementSize = sizeof(float); buffer.size = buffer.elementSize * count;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ARRAY_BUFFER, buffer.id); //select the buffer
  glBufferData(GL_ARRAY_BUFFER, buffer.size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return buffer;
}

index_buffer CreateIndexBuffer(unsigned int *data, unsigned int count)
{
  index_buffer buffer; buffer.count = count;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.count * sizeof(unsigned int), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return buffer;
}

vertex_array CreateVertexArray()
{
  vertex_array vertexArray;
  glGenVertexArrays(1, &vertexArray.id);
  return vertexArray;
}

buffer_layout CreateBufferLayout()
{
  buffer_layout bufferLayout = {};
  return bufferLayout;
}

void Init(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  char stringBuffer[260];

  engineState->vertexArray = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer(positions, 8); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout
  bufferLayout.push(2, GL_FLOAT); //describe the buffer layout
  engineState->vertexArray.addBuffer(vertexBuffer, bufferLayout); //describe the vao
  engineState->indexBuffer = CreateIndexBuffer(indices, 6);

  read_file_result f1 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.vert", stringBuffer, 260));
  read_file_result f2 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.frag", stringBuffer, 260));
  shader sh = CreateShader((char *)f1.content, (char *)f2.content);

  engineState->defaultMaterial.sh = sh;
  engineState->defaultMaterial.setColor(0.5f, 0.3f, 1.0f, 1.0f);
}
void Update(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  Clear();
  Draw(engineState->vertexArray, engineState->indexBuffer, engineState->defaultMaterial);
}
