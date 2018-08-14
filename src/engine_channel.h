/* dependencies
platform.h
engine.h
maccis_file_io.h
backend.h
maccis_user_input.h
*/

struct engine_memory
{
  void *storage;
  long int storageSize;
  char *maccisDirectory;
  platform_read_file *ReadFile;
  platform_free_file *FreeFile;
  platform_get_clock *StartClock;
  platform_get_clock *EndClock;
  platform_get_delta_time *GetClockDeltaTime;
  platform_log *Log;
};

#define CLEAR(name) void name()
typedef CLEAR(renderer_clear);

#define DRAW(name) void name(game_object object, camera cam)
typedef DRAW(renderer_draw);

struct renderer
{
  renderer_clear *Clear;
  renderer_draw *Draw;
};

#define GAME_OBJECT_FROM_RAW_MODEL(name) game_object name(raw_model model, shader shader, texture texture)
typedef GAME_OBJECT_FROM_RAW_MODEL(engine_game_object_from_raw_model);

struct engine
{
  engine_game_object_from_raw_model *GameObjectFromRawModel;
};

#define GAME_INIT(name) void name(engine *engine, engine_memory *memory)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name(engine *engine, renderer *renderer, engine_memory *memory, user_input *userInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_CLOSE(name) void name()
typedef GAME_CLOSE(game_close);

struct game_code
{
  game_init *GameInit;
  game_update_and_render *GameUpdateAndRender;
  game_close *GameClose;
  bool isValid;
};
