/*
TODO(Noah):
-multithreading
-sound
-all header files should follow the layers architecture as opposed to the rats nest architecture
-support multiple graphics API
-remove openGL calls in engine.h
-implement pseudo-game for the support of different test frameworks
-implement a main menu for selecting through the various test frameworks!
-implement fonts aligning with the left edge
-implement a basic debugger which prints to the top left using our new font system
-fix that thing where the A gets cut off
-add transparency to the rendering of text
*/

#include <maccis_math.h>
#include <engine.h>

#include <maccis_file_io.h>
#include<engine_utility.cpp> //service to engine and all other services
#include <renderer.cpp> //service to engine
#include <file_io.cpp> //service to engine

#include <maccis_asset.h>
#include <asset.cpp> //service to engine

//TODO(Noah): manage what goes in this hook
#include <engine_hook.cpp> //service that recieves information from the other services

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

texture CreateTexture(platform_read_file *ReadFile, platform_free_file *FreeFile, char *path, unsigned int slot)
{
  texture newTexture;
  glGenTextures(1, &newTexture.id);
  glBindTexture(GL_TEXTURE_2D, newTexture.id);
  newTexture.localTexture = LoadBMPImage(ReadFile, path);

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
  memcpy(result.matp, identity, sizeof(float) * 16);
  return result;
}

INTERNAL mat4 CreateOrthographicMatrix(float width, float height, float n, float f)
{
  mat4 result = {};
  float r = width; float l = 0;
  float t = height; float b = 0;
  float identity[] = {
    2 / (r - l), 0, 0, 0,
    0, 2 / (t -b),  0, 0,
    0, 0, -2 / (f - n),0,
    -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n),1
  };
  memcpy(result.matp, identity, sizeof(float) * 16);
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

INTERNAL camera CreateOrthoCamera(float width, float height)
{
  camera cam = {};
  cam.proj = CreateOrthographicMatrix(width, height, 1.0f, 100.0f);
  cam.trans.setScale(1.0f, 1.0f , 1.0f);
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
  newTexture.localTexture = BMPToImage(bitmap);

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

INTERNAL renderable_2D CreateSprite(float uniformScale, vec2 pos, float width, float height)
{
  renderable_2D renderable = {};

  //setup transform of renderable
  renderable.scale = NewVec2(uniformScale, uniformScale);

  //generate vertices of the renderable
  renderable.vertices[0].position[0] = pos.x;
  renderable.vertices[0].position[1] = pos.y;
  renderable.vertices[0].textureCoordinate[0] = 0.0f;
  renderable.vertices[0].textureCoordinate[1] = 0.0f;

  renderable.vertices[1].position[0] = pos.x + width * uniformScale;
  renderable.vertices[1].position[1] = pos.y;
  renderable.vertices[1].textureCoordinate[0] = 1.0f;
  renderable.vertices[1].textureCoordinate[1] = 0.0f;

  renderable.vertices[2].position[0] = pos.x + width * uniformScale;
  renderable.vertices[2].position[1] = pos.y + height * uniformScale;
  renderable.vertices[2].textureCoordinate[0] = 1.0f;
  renderable.vertices[2].textureCoordinate[1] = 1.0f;

  renderable.vertices[3].position[0] = pos.x;
  renderable.vertices[3].position[1] = pos.y + height * uniformScale;
  renderable.vertices[3].textureCoordinate[0] = 0.0f;
  renderable.vertices[3].textureCoordinate[1] = 1.0f;

  return renderable;
}

INTERNAL renderable_2D CreateSpriteFromTexture(float uniformScale, vec2 pos,
  texture tex)
{
  renderable_2D renderable = {};

  //setup transform of renderable
  renderable.scale = NewVec2(uniformScale, uniformScale);
  renderable.width = tex.localTexture.width;
  renderable.height = tex.localTexture.height;

  //generate vertices of the renderable
  renderable.vertices[0].position[0] = pos.x - tex.localTexture.width * uniformScale / 2.0f;
  renderable.vertices[0].position[1] = pos.y - tex.localTexture.height * uniformScale / 2.0f;
  renderable.vertices[0].textureCoordinate[0] = 0.0f;
  renderable.vertices[0].textureCoordinate[1] = 0.0f;

  renderable.vertices[1].position[0] = pos.x + tex.localTexture.width * uniformScale / 2.0f;
  renderable.vertices[1].position[1] = pos.y - tex.localTexture.height * uniformScale / 2.0f;
  renderable.vertices[1].textureCoordinate[0] = 1.0f;
  renderable.vertices[1].textureCoordinate[1] = 0.0f;

  renderable.vertices[2].position[0] = pos.x + tex.localTexture.width * uniformScale / 2.0f;
  renderable.vertices[2].position[1] = pos.y + tex.localTexture.height * uniformScale / 2.0f;
  renderable.vertices[2].textureCoordinate[0] = 1.0f;
  renderable.vertices[2].textureCoordinate[1] = 1.0f;

  renderable.vertices[3].position[0] = pos.x - tex.localTexture.width * uniformScale / 2.0f;
  renderable.vertices[3].position[1] = pos.y + tex.localTexture.height * uniformScale / 2.0f;
  renderable.vertices[3].textureCoordinate[0] = 0.0f;
  renderable.vertices[3].textureCoordinate[1] = 1.0f;

  return renderable;
}

void LoadFontFromAsset(memory_arena *arena, loaded_asset asset, loaded_font *outFont)
{
  asset_wrapper wrappers[4];
  ParseAsset(&asset, wrappers);

  loaded_font *sourceFont = (loaded_font *)wrappers[2].asset;
  *outFont = *sourceFont;
  outFont->horizontalAdvance = (float  *)wrappers[3].asset;

  /*///////////
  FILE *file = fopen("C:\\dev\\log.txt","w+");
  for (unsigned int i = 0; i < outFont->codePointCount; i++)
  {
    for (unsigned int j = 0; j < outFont->codePointCount; j++)
    {
      float f = outFont->horizontalAdvance[i * outFont->codePointCount + j];
      fprintf(file, "letter 1: %d, letter 2: %d, f: %f\n",i + outFont->firstChar, j + outFont->firstChar, f);
    }
  }
  fclose(file);
  ////////////////*/

  outFont->fontSprites = arena->push(outFont->codePointCount * sizeof(renderable_2D));
  renderable_2D *fontSprites = (renderable_2D *)outFont->fontSprites;

  character_desriptor *descriptors = (character_desriptor *)wrappers[1].asset;
  unsigned int descriptorCount = wrappers[1].assetSize / sizeof(character_desriptor);
  for (unsigned int i = 0; i < descriptorCount; i++)
  {
    character_desriptor descriptor = descriptors[i];
    fontSprites[i] = CreateSprite(1.0f, NewVec2(0.0f, 0.0f), descriptor.width, descriptor.height);
    fontSprites[i].width = descriptor.width;
    fontSprites[i].height = descriptor.height;
    fontSprites[i].alignPercentage[0] = descriptor.alignPercentage[0];
    fontSprites[i].alignPercentage[1] = descriptor.alignPercentage[1];
    fontSprites[i].vertices[0].textureCoordinate[0] = descriptor.textureCoordinate[0];
    fontSprites[i].vertices[0].textureCoordinate[1] = descriptor.textureCoordinate[1];
    fontSprites[i].vertices[1].textureCoordinate[0] = descriptor.textureCoordinate[2];
    fontSprites[i].vertices[1].textureCoordinate[1] = descriptor.textureCoordinate[3];
    fontSprites[i].vertices[2].textureCoordinate[0] = descriptor.textureCoordinate[4];
    fontSprites[i].vertices[2].textureCoordinate[1] = descriptor.textureCoordinate[5];
    fontSprites[i].vertices[3].textureCoordinate[0] = descriptor.textureCoordinate[6];
    fontSprites[i].vertices[3].textureCoordinate[1] = descriptor.textureCoordinate[7];
  }
}

void Init(engine_memory memory, unsigned int width, unsigned int height)
{
  engine_state *engineState = (engine_state *)memory.storage;
  char stringBuffer[260];

  //open font
  engineState->fontAsset = LoadAsset(memory.ReadFile, memory.FreeFile,
    &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\font.asset",stringBuffer));

  //load in the 2D shader
  read_file_result f1 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\2D_shader.vert", stringBuffer));
  read_file_result f2 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\2D_shader.frag", stringBuffer));
  shader sh1 = CreateShader((char *)f1.content, (char *)f2.content);

  //push memory for 2d batch renderer
  engineState->batchRenderer2D = (batch_renderer_2D *)engineState->memoryArena.push(sizeof(batch_renderer_2D));

  //initialize 2d batch renderer
  InitializeBatchRenderer2D(engineState->batchRenderer2D, sh1);

  //create cameras
  engineState->guiCamera = CreateOrthoCamera((float)width, (float)height);
  engineState->mainCamera = CreateCamera((float)width, (float)height, 90.0f);

  {
    engineState->defaultObject.mesh.vao = CreateVertexArray(); //make the vao
    vertex_buffer vertexBuffer = CreateVertexBuffer(vertices, 16); //make the vertex buffer
    buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout
    bufferLayout.push(2, GL_FLOAT); //describe the buffer layout
    bufferLayout.push(2, GL_FLOAT);
    engineState->defaultObject.mesh.vao.addBuffer(vertexBuffer, bufferLayout); //describe the vao
    engineState->defaultObject.mesh.indexBuffer = CreateIndexBuffer(indices, 6);
  }

  //load in the 3D shader
  read_file_result f3 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.vert", stringBuffer));
  read_file_result f4 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.frag", stringBuffer));
  shader sh2 = CreateShader((char *)f3.content, (char *)f4.content);

  //set material of deault object
  engineState->defaultObject.material.sh = sh2;
  engineState->defaultObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);

  //create textures
  engineState->defaultTexture = CreateTexture(memory.ReadFile, memory.FreeFile,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\test.bmp", stringBuffer), 0);
  loaded_bitmap *fontAtlasBitmap = (loaded_bitmap *)engineState->fontAsset.pWrapper->asset;
  engineState->fontAtlas = BuildTextureFromBitmapNoFree(*fontAtlasBitmap, 1);

  //create font sprites!
  LoadFontFromAsset(&engineState->memoryArena, engineState->fontAsset, &engineState->font);
  engineState->batchRenderer2D->textureAtlas = engineState->fontAtlas;

  engineState->defaultObject.material.setTexture(engineState->defaultTexture);
  engineState->defaultObject.transform.setScale(1.0f, 1.0f, 1.0f);
  engineState->defaultObject.transform.setPosition(0.0f, 0.0f, -5.0f);

  memory.StartClock();

  engineState->monkeyAsset = LoadAsset(memory.ReadFile, memory.FreeFile,
    &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\monkey2.asset", stringBuffer));
  raw_model model = *(raw_model *)engineState->monkeyAsset.pWrapper->asset;
  engineState->suzanne = GameObjectFromRawModel(model, sh2);

  memory.EndClock();
  float duration = memory.GetClockDeltaTime();
  printf("obj time: %fms\n", duration * 1000.0f);

  engineState->suzanne.material.setTexture(engineState->defaultTexture);
  engineState->character = '?';
}

void Update(engine_memory memory, user_input userInput)
{
  memory.StartClock();
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
  if (userInput.keyStates[MACCIS_MOUSE_LEFT].endedDown)
  {
    engineState->mainCamera.rotate(0.0f, userInput.mouseDX / 5, 0.0f);
    engineState->mainCamera.rotate(-userInput.mouseDY / 5, 0.0f, 0.0f);
  }

  //render 3d scene
  Clear();
  engineState->defaultObject.transform.rotate(0.0f, -2.0f, 0.0f);
  Draw(engineState->defaultObject, engineState->mainCamera);
  engineState->suzanne.transform.rotate(0.0f, 2.0f, 0.0f);
  Draw(engineState->suzanne, engineState->mainCamera);

  //render gui
  BeginBatchRenderer2D(engineState->batchRenderer2D);

  //Submit(engineState->batchRenderer2D, engineState->fontSprites[GetIndexFromCharacter(engineState->character)]);
  DebugPushText("GameUpdateAndRender", engineState->batchRenderer2D, &engineState->font);

  EndBatchRenderer2D(engineState->batchRenderer2D);
  Flush(engineState->batchRenderer2D, engineState->guiCamera);

  memory.EndClock();
  float duration = memory.GetClockDeltaTime();
  printf("update time: %fms\n", duration * 1000.0f);
  engineState->elapsedTime += duration;
  if (engineState->elapsedTime > 0.8f)
  {
    engineState->character += 1;
    engineState->elapsedTime = 0.0f;
    if (engineState->character == 'u')
    {
      engineState->character = '?';
    }
  }
}

void Clean(engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  engineState->defaultTexture.del();
  memory.FreeFile(memory.storage);
}
