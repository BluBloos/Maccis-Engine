extern "C" GAMEINIT(GameInit)
{
  //here is where you can put any initialization code
  engineState->monkeyAsset = LoadAsset(memory.ReadFile, memory.FreeFile, &engineState->memoryArena,
    MaccisCatStringsUnchecked(memory.maccisDirectory, "res\\monkey2.asset", stringBuffer));
  raw_model model = *(raw_model *)engineState->monkeyAsset.pWrapper->asset;
  engineState->suzanne = GameObjectFromRawModel(model, engineState->defaultShader);
  engineState->suzanne.material.setTexture(engineState->defaultTexture);
}

extern "C" GAMEUPDATEANDRENDER(GameUpdateAndRender)
{
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

  Clear();
  engineState->suzanne.transform.rotate(0.0f, 2.0f, 0.0f);
  Draw(engineState->suzanne, engineState->mainCamera);
}

extern "C" GAMECLOSE(GameClose)
{
  //here is where you can do things when the game is finalizing
}
