#include <maccis.h>
#include <maccis_math.h>
#include <renderer.cpp>
#include <engine.h>


INTERNAL float vertices[] = {
  -0.5f, -0.5f, 0.0f, 0.0f, //0
   0.5f, -0.5f, 1.0f, 0.0f, //1
   0.5f,  0.5f, 1.0f, 1.0f, //2
  -0.5f,  0.5f, 0.0f, 1.0f  //3
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

loaded_bitmap LoadBMP(platform_read_file *ReadFile, char *path)
{
	loaded_bitmap bitmap = {};
	bitmap.scale = 1;
	read_file_result fileResult = ReadFile(path);
	if (fileResult.contentSize != 0)
	{
		bitmap_header *header = (bitmap_header *)fileResult.content;
		bitmap.pixelPointer = (unsigned int *) ( (unsigned char *)fileResult.content + header->BitmapOffset );
		bitmap.height = header->Height;
		bitmap.width = header->Width;
    bitmap.container = fileResult.content;
	}
	return bitmap;
}

texture CreateTexture(platform_read_file *ReadFile, platform_free_file *FreeFile, char *path)
{
  texture newTexture;
  glGenTextures(1, &newTexture.id);
  glBindTexture(GL_TEXTURE_2D, newTexture.id);
  newTexture.localTexture = LoadBMP(ReadFile, path);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newTexture.localTexture.width,
    newTexture.localTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newTexture.localTexture.pixelPointer);
  glBindTexture(GL_TEXTURE_2D, 0);
  newTexture.localTexture.free(FreeFile);
  return newTexture;
}

mat4 CreateProjectionMatrix(float fov, float aspectRatio, float n, float f)
{
  mat4 result = {};

  float r = tanf(fov * DEGREES_TO_RADIANS / 2.0f) * n;
  float l = -r;
  float t = r / aspectRatio;
  float b = -t;

  float identity[] = {
    2 * n / (r - l),0,0,0,
    0,2 * n / (t -b),0,0,
    (r + l) / (r -l), (t + b) / (t -b), -(f + n) / (f -n), -1,
    0,0,-2 * f * n / (f -n),0
  };
  for (unsigned int x = 0; x < 16; x++)
  {
    result.matp[x] = identity[x];
  }

  return result;
}

INTERNAL camera CreateCamera(float width, float height, float fov)
{
  camera cam;
  cam.proj = CreateProjectionMatrix(fov, width / height, 1.0f, 5.0f);
  return cam;
}

void Init(engine_memory memory, unsigned int width, unsigned int height)
{
  engine_state *engineState = (engine_state *)memory.storage;
  char stringBuffer[260];

  engineState->defaultObject.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer(vertices, 16); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout
  bufferLayout.push(2, GL_FLOAT); //describe the buffer layout
  bufferLayout.push(2, GL_FLOAT);
  engineState->defaultObject.vao.addBuffer(vertexBuffer, bufferLayout); //describe the vao
  engineState->defaultObject.indexBuffer = CreateIndexBuffer(indices, 6);

  read_file_result f1 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.vert", stringBuffer, 260));
  read_file_result f2 = memory.ReadFile(BuildFilePath(memory.maccisDirectory, "src\\shader.frag", stringBuffer, 260));
  shader sh = CreateShader((char *)f1.content, (char *)f2.content);

  engineState->defaultObject.material.sh = sh;
  engineState->defaultObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);

  engineState->defaultTexture = CreateTexture(memory.ReadFile, memory.FreeFile,
    BuildFilePath(memory.maccisDirectory, "res\\test.bmp", stringBuffer, 260));
  engineState->defaultTexture.bind(0);
  engineState->defaultObject.material.setTexture(0);

  engineState->mainCamera = CreateCamera((float)width, (float)height, 90.0f);
  engineState->defaultObject.transform.setScale(1.0f, 1.0f, 1.0f);
  engineState->defaultObject.transform.setPosition(0.0f, 0.0f, -1.0f);
}

void Update(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  Clear();
  engineState->defaultObject.material.setColor(engineState->r++ / 255.0f, 1.0f, 1.0f, 1.0f);
  //engineState->defaultObject.transform.scale(1.01f, 1.0f, 1.0f);
  engineState->defaultObject.transform.rotate(0.0f, 2.0f, 0.0f);
  engineState->defaultObject.transform.translate(0.0f, 0.0f, -0.01f);
  Draw(engineState->defaultObject, engineState->mainCamera);
}

void Clean(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->defaultTexture.del();
  memory.FreeFile(memory.storage);
}
