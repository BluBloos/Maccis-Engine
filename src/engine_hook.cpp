/* dependencies
maccis.h
backend.h
engine.h
engine_channel.h
*/

struct engine_state
{
  //NOTE(Noah): used for global timing, but its only a float so it loosed precision
  float elapsedTime;
  //char character;
  //TODO(Noah): need the ability to free allocated memory on memoryArena
  memory_arena memoryArena;
  texture defaultTexture;
  shader defaultShader;
  //texture fontAtlas;
  //game_object defaultObject;
  game_object suzanne;
  camera mainCamera;
  renderer renderer;
  engine engine;
  //camera guiCamera;
  //loaded_bitmap font[100];
  //loaded_asset fontAsset;
  loaded_asset monkeyAsset;
  //NOTE(Noah): unless otherwise specified by the game the batch renderer is not initialized
  //thats why it's a pointer
  //batch_renderer_2D *batchRenderer2D;
  //loaded_font font;
};
