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
#include <engine.h>

#include <maccis_file_io.h>
#include <engine_channel.h>

#include <file_io.cpp> //service to engine
#include <engine_utility.cpp> //service to engine and all other services

#include <maccis_strings.h>
#include <renderer.cpp> //service to engine

#include <maccis_asset.h>
#include <asset.cpp> //service to engine

#include <game.h>

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

void Init(game_code gameCode, engine_memory memory, unsigned int width, unsigned int height)
{
  engine_state *engineState = (engine_state *)memory.storage;
  char stringBuffer[260];

  //create the default shader
  read_file_result f1 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.vert", stringBuffer));
  read_file_result f2 = memory.ReadFile(MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\shader.frag", stringBuffer));
  engineState->defaultShader = CreateShader((char *)f1.content, (char *)f2.content);

  //create the main camera
  engineState->mainCamera = CreateCamera((float)width, (float)height, 90.0f);

  //create the default texture
  loaded_bitmap bitmap = LoadBMP(memory.ReadFile, MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\checker.bmp", stringBuffer));
  engineState->defaultTexture = CreateTexture(bitmap.pixelPointer, bitmap.width, bitmap.height, 0);
  FreeBitmap(memory.FreeFile, bitmap);

  //create the renderer
  engineState->renderer.Clear = Clear;
  engineState->renderer.Draw = Draw;
  engineState->renderer.InitializeBatchRenderer2D = InitializeBatchRenderer2D;
  engineState->renderer.BeginBatchRenderer2D = BeginBatchRenderer2D;
  engineState->renderer.EndBatchRenderer2D = EndBatchRenderer2D;
  engineState->renderer.Flush = Flush;

  //create the engine
  engineState->engine.GameObjectFromRawModel = GameObjectFromRawModel;
  engineState->engine.CreateTexture = CreateTexture;
  engineState->engine.CreateShader = CreateShader;

  memory.EndClock();
  if(gameCode.isValid)
  {
    gameCode.GameInit(&engineState->engine, &engineState->renderer, &memory, width, height);
  }
  #if 0
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
  #endif
}

void Update(game_code gameCode, engine_memory memory, user_input userInput)
{
  engine_state *engineState = (engine_state *)memory.storage;
  #if 0
  //render gui
  BeginBatchRenderer2D(engineState->batchRenderer2D);

  //Submit(engineState->batchRenderer2D, engineState->fontSprites[GetIndexFromCharacter(engineState->character)]);
  DebugPushText("GameUpdateAndRender", engineState->batchRenderer2D, &engineState->font);

  EndBatchRenderer2D(engineState->batchRenderer2D);
  Flush(engineState->batchRenderer2D, engineState->guiCamera);

  memory.EndClock();
  float duration = memory.GetClockDeltaTime();
  //TODO(Noah): fix this log here
  //Log(FloatToString(duration * 1000.0f));
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
  #endif
  if(gameCode.isValid)
  {
    gameCode.GameUpdateAndRender(&engineState->engine, &engineState->renderer, &memory, &userInput);
  }
}

void Clean(game_code gameCode, engine_memory memory)
{
  engine_state *engineState = (engine_state *)memory.storage;
  DeleteTexture(&engineState->defaultTexture);
  //memory.FreeFile(memory.storage);
  //memory.GameCode.GameClose();
}
