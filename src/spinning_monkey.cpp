#include <platform.h>
#include <maccis_user_input.h>
#include <maccis_math.h>
#include <string.h>
#include <backend.h>
#include <maccis.h>

#include <engine.h>

#include <maccis_file_io.h>
#include <engine_channel.h>
//#include <file_io.cpp> //service to engine
//#include <engine_utility.cpp> //service to engine and all other services
#include <maccis_strings.h>

#include <maccis_asset.h>
#include <asset.cpp> //service to engine

#include <renderer_inline.cpp>
//#include <pushbuffer.cpp>

#include <engine_hook.cpp> //service that recieves information from the other services

extern "C" GAME_INIT(GameInit)
{
  char stringBuffer[256];
  engine_state *engineState = (engine_state *)memory->storage;

  engineState->monkeyAsset = LoadAsset(memory->ReadFile, memory->FreeFile, &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory->maccisDirectory, "res\\monkey2.asset", stringBuffer));
  raw_model model = *(raw_model *)engineState->monkeyAsset.pWrapper->asset;

  engineState->suzanne = engine->GameObjectFromRawModel(model, engineState->defaultShader, engineState->defaultTexture);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  engine_state *engineState = (engine_state *)memory->storage;
  //process input
  float speed = 5 / 60.0f;
  if (userInput->keyStates[MACCIS_KEY_W].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, 0.0f, -speed);
  }
  if (userInput->keyStates[MACCIS_KEY_A].endedDown)
  {
    engineState->mainCamera.translateLocal(-speed, 0.0f, 0.0f);
  }
  if (userInput->keyStates[MACCIS_KEY_S].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, 0.0f, speed);
  }
  if (userInput->keyStates[MACCIS_KEY_D].endedDown)
  {
    engineState->mainCamera.translateLocal(speed, 0.0f, 0.0f);
  }
  if (userInput->keyStates[MACCIS_KEY_SHIFT].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, -speed, 0.0f);
  }
  if (userInput->keyStates[MACCIS_KEY_SPACE].endedDown)
  {
    engineState->mainCamera.translateLocal(0.0f, speed, 0.0f);
  }
  if (userInput->keyStates[MACCIS_MOUSE_LEFT].endedDown)
  {
    engineState->mainCamera.rotate(0.0f, userInput->mouseDX / 5, 0.0f);
    engineState->mainCamera.rotate(-userInput->mouseDY / 5, 0.0f, 0.0f);
  }

  renderer->Clear();
  engineState->suzanne.transform.rotate(0.0f, 2.0f, 0.0f);
  renderer->Draw(engineState->suzanne, engineState->mainCamera);
}

extern "C" GAME_CLOSE(GameClose)
{
  //here is where you can do things when the game is finalizing
}
