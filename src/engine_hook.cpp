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
  //loaded_bitmap font[100];
  loaded_asset fontAsset;
  loaded_asset monkeyAsset;
  //NOTE(Noah): unless otherwise specified by the game the batch renderer is not initialized
  //thats why it's a pointer
  batch_renderer_2D *batchRenderer2D;
  loaded_font font;
};
