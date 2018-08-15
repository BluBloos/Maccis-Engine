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

#define INITIALIZE_BATCH_RENDERER_2D(name) void name(batch_renderer_2D *batchRenderer2D, shader defaultShader)
typedef INITIALIZE_BATCH_RENDERER_2D(renderer_initialize_batch_renderer_2D);

#define BEGIN_BATCH_RENDERER_2D(name) void name(batch_renderer_2D *batchRenderer2D)
typedef BEGIN_BATCH_RENDERER_2D(renderer_begin_batch_renderer_2D);

#define END_BATCH_RENDERER_2D(name) void name(batch_renderer_2D *batchRenderer2D)
typedef END_BATCH_RENDERER_2D(renderer_end_batch_renderer_2D);

#define FLUSH(name) void name(batch_renderer_2D *batchRenderer2D, camera cam)
typedef FLUSH(renderer_flush);

struct renderer
{
  renderer_clear *Clear;
  renderer_draw *Draw;
  renderer_initialize_batch_renderer_2D *InitializeBatchRenderer2D;
  renderer_begin_batch_renderer_2D *BeginBatchRenderer2D;
  renderer_end_batch_renderer_2D *EndBatchRenderer2D;
  renderer_flush *Flush;
};

#define GAME_OBJECT_FROM_RAW_MODEL(name) game_object name(raw_model model, shader shader, texture texture)
typedef GAME_OBJECT_FROM_RAW_MODEL(game_object_from_raw_model);

#define CREATE_TEXTURE(name) texture name(unsigned int *pixelPointer, unsigned int width, unsigned int height, unsigned int slot)
typedef CREATE_TEXTURE(create_texture);

#define CREATE_SHADER(name) shader name(char *vertexShader, char *fragmentShader);
typedef CREATE_SHADER(create_shader);

struct engine
{
  game_object_from_raw_model *GameObjectFromRawModel;
  create_texture *CreateTexture;
  create_shader *CreateShader;
};

#define GAME_INIT(name) void name(engine *engine, renderer *renderer, engine_memory *memory, unsigned int width, unsigned int height)
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
