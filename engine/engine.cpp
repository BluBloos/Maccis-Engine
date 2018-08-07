#include <engine.h>

#include <renderer.cpp> //service to engine
#include <file_io.cpp> //service to engine
#include <asset.cpp> //service to engine

#include <iostream>
#include <chrono>

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

texture CreateTexture(platform_read_file *ReadFile, platform_free_file *FreeFile, char *path, unsigned int slot)
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
  newTexture.slot = slot;
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

INTERNAL texture BuildTextureFromBitmapNoFree(loaded_bitmap bitmap, unsigned int slot)
{
  texture newTexture = {};
  glGenTextures(1, &newTexture.id);
  glBindTexture(GL_TEXTURE_2D, newTexture.id);
  newTexture.localTexture = bitmap;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newTexture.localTexture.width,
    newTexture.localTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newTexture.localTexture.pixelPointer);
  glBindTexture(GL_TEXTURE_2D, 0);
  newTexture.slot = slot;
  return newTexture;
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
  gameObject.transform.setScale(1.0f, 1.0f, 1.0f);
  gameObject.transform.setPosition(0.0f, 0.0f, -1.0f);

  return gameObject;
}

loaded_bitmap GetCharacterFromFont(loaded_bitmap *font, unsigned int character)
{
  int index = character - 32;
  if (index > 0)
  {
    return font[index];
  } else
  {
    return font[0];
  }
}

void Init(engine_memory memory, unsigned int width, unsigned int height)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->memoryArena.init((char *)memory.storage + sizeof(engine_state), memory.storageSize - sizeof(engine_state));
  char stringBuffer[260];

  //open font
  engineState->fontAsset = LoadAsset(memory.ReadFile, memory.FreeFile,
    &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\arial.asset",stringBuffer));

  //load bitmaps from asset into bitmap list
  ParseAssetOfBitmapList(&engineState->fontAsset, engineState->font);

  engineState->defaultObject.mesh.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer(vertices, 16); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout
  bufferLayout.push(2, GL_FLOAT); //describe the buffer layout
  bufferLayout.push(2, GL_FLOAT);
  engineState->defaultObject.mesh.vao.addBuffer(vertexBuffer, bufferLayout); //describe the vao
  engineState->defaultObject.mesh.indexBuffer = CreateIndexBuffer(indices, 6);

  read_file_result f1 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.vert", stringBuffer));
  read_file_result f2 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.frag", stringBuffer));
  shader sh = CreateShader((char *)f1.content, (char *)f2.content);

  engineState->defaultObject.material.sh = sh;
  engineState->defaultObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);

  engineState->defaultTexture = CreateTexture(memory.ReadFile, memory.FreeFile,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\test.bmp", stringBuffer), 0);

  engineState->testTexture = BuildTextureFromBitmapNoFree(
    GetCharacterFromFont(engineState->font, 'A'), 1);

  engineState->defaultObject.material.setTexture(engineState->testTexture);

  engineState->mainCamera = CreateCamera((float)width, (float)height, 90.0f);
  engineState->defaultObject.transform.setScale(1.0f, 1.0f, 1.0f);
  engineState->defaultObject.transform.setPosition(0.0f, 0.0f, -5.0f);

  for (unsigned int i = 0; i < 10; i++)
  {
    engineState->dummyObjects[i].setScale(1.0f, 1.0f, 1.0f);
  }

  engineState->dummyObjects[0].setPosition(-10.0f, 0.0f, -25.0f);
  engineState->dummyObjects[1].setPosition(+10.0f, 0.0f, -20.0f);
  engineState->dummyObjects[2].setPosition(0.0f, -5.0f, -15.0f);
  engineState->dummyObjects[3].setPosition(0.0f, +5.0f, -10.0f);

  auto start = std::chrono::high_resolution_clock::now();

  #if 0
  engineState->suzanne = GameObjectFromRawModel(LoadOBJ(&engineState->memoryArena, memory.maccisDirectory,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\metaball.obj", stringBuffer)), sh);
  #else
  engineState->monkeyAsset = LoadAsset(memory.ReadFile, memory.FreeFile,
    &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\monkey.asset", stringBuffer));
  raw_model model = *(raw_model *)engineState->monkeyAsset.pWrapper->asset;
  engineState->suzanne = GameObjectFromRawModel(model, sh);
  #endif

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> duration = end - start;
  printf("obj time: %fms\n", duration.count() * 1000.0f);

  engineState->suzanne.material.setTexture(engineState->defaultTexture);
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
    engineState->mainCamera.rotate(0.0f, userInput.mouseDX / 5, 0.0f);
    engineState->mainCamera.rotate(-userInput.mouseDY / 5, 0.0f, 0.0f);
  }

  //do rendering
  Clear();

  engineState->defaultObject.transform.rotate(0.0f, -2.0f, 0.0f);
  Draw(engineState->defaultObject, engineState->mainCamera);

  engineState->suzanne.transform.rotate(0.0f, 2.0f, 0.0f);
  Draw(engineState->suzanne, engineState->mainCamera);
}

void Clean(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->defaultTexture.del();
  memory.FreeFile(memory.storage);
}
