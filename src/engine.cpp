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

INTERNAL unsigned int CompileShader(unsigned int type, char *shader)
{
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &shader, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    //TODO(Noah): remove _alloca and add logging functions provided by the platform layer
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)_alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    printf("Failed to comile shader!\n");
    printf("%s\n", message);
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

vertex_buffer CreateVertexBuffer(float *data, unsigned int floatCount)
{
  vertex_buffer buffer = {}; buffer.elementSize = sizeof(float); buffer.size = buffer.elementSize * floatCount;
  glGenBuffers(1, &buffer.id);
  glBindBuffer(GL_ARRAY_BUFFER, buffer.id); //select the buffer
  glBufferData(GL_ARRAY_BUFFER, buffer.size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return buffer;
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
  camera cam = {};
  cam.proj = CreateProjectionMatrix(fov, width / height, 1.0f, 100.0f);
  cam.trans.setScale(1.0f, 1.0f, 1.0f);
  cam.trans.forward = NewVec3(0.0f, 0.0f, 1.0f);
  cam.trans.up = NewVec3(0.0f, 1.0f, 0.0f);
  cam.trans.right = NewVec3(1.0f, 0.0f, 0.0f);
  return cam;
}

game_object GameObjectFromRawModel(raw_model model, shader sh)
{
  game_object gameObject = {};

  gameObject.mesh.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer((float *)model.vertices, model.vertexCount * 8); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout

  bufferLayout.push(3, GL_FLOAT); //describe the buffer layout
  bufferLayout.push(2, GL_FLOAT);
  bufferLayout.push(3, GL_FLOAT);

  gameObject.mesh.vao.addBuffer(vertexBuffer, bufferLayout); //describe the vao
  gameObject.mesh.indexBuffer = CreateIndexBuffer((unsigned int *)model.indices, model.indexCount);

  gameObject.material.sh = sh;
  gameObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gameObject.material.setTexture(0);
  gameObject.transform.setScale(1.0f, 1.0f, 1.0f);
  gameObject.transform.setPosition(0.0f, 0.0f, -1.0f);

  return gameObject;
}

void Init(engine_memory memory, unsigned int width, unsigned int height)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->memoryArena.init((char *)memory.storage + sizeof(engine_state), memory.storageSize - sizeof(engine_state));
  char stringBuffer[260];

  engineState->defaultObject.mesh.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer(vertices, 16); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout
  bufferLayout.push(2, GL_FLOAT); //describe the buffer layout
  bufferLayout.push(2, GL_FLOAT);
  engineState->defaultObject.mesh.vao.addBuffer(vertexBuffer, bufferLayout); //describe the vao
  engineState->defaultObject.mesh.indexBuffer = CreateIndexBuffer(indices, 6);

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

  for (unsigned int i = 0; i < 10; i++)
  {
    engineState->dummyObjects[i].setScale(1.0f, 1.0f, 1.0f);
  }

  engineState->dummyObjects[0].setPosition(-10.0f, 0.0f, -25.0f);
  engineState->dummyObjects[1].setPosition(+10.0f, 0.0f, -20.0f);
  engineState->dummyObjects[2].setPosition(0.0f, -5.0f, -15.0f);
  engineState->dummyObjects[3].setPosition(0.0f, +5.0f, -10.0f);

  engineState->suzanne = GameObjectFromRawModel(LoadOBJ(engineState->memoryArena, memory.maccisDirectory,
    BuildFilePath(memory.maccisDirectory, "res\\monkey.obj", stringBuffer, 260)), sh);
}

void Update(engine_memory memory, user_input userInput)
{
  engine_state *engineState = (engine_state *)memory.storage;

  //process input
  float speed = 5 / 60.0f;
  if (userInput.keyStates[MACCIS_KEY_W].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, 0.0f, -speed);
  }
  if (userInput.keyStates[MACCIS_KEY_A].endedDown)
  {
    engineState->mainCamera.translateLocal(-speed, 0.0f, 0.0f);
  }
  if (userInput.keyStates[MACCIS_KEY_S].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, 0.0f, speed);
  }
  if (userInput.keyStates[MACCIS_KEY_D].endedDown)
  {
    engineState->mainCamera.translateLocal(speed, 0.0f, 0.0f);
  }
  if (userInput.keyStates[MACCIS_KEY_SHIFT].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, -speed, 0.0f);
  }
  if (userInput.keyStates[MACCIS_KEY_SPACE].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, speed, 0.0f);
  }
  if (userInput.keyStates[MACCIS_MOUSE_MIDDLE].endedDown)
  {
    engineState->mainCamera.rotate(0.0f, userInput.mouseDX / 10, 0.0f);
    engineState->mainCamera.rotate(userInput.mouseDY / 10, 0.0f, 0.0f);
  }

  //do rendering
  Clear();

  engineState->suzanne.transform.rotate(0.0f, 2.0f, 0.01f);
  //engineState->suzanne.transform.translate(0.0f, 0.0f, -0.01f);
  Draw(engineState->suzanne, engineState->mainCamera);
}

void Clean(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->defaultTexture.del();
  memory.FreeFile(memory.storage);
}
