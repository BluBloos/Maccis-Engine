//zero dependencies

#define INTERNAL static
#define PERSISTENT static

#define KB(number) number * 1000;
#define MB(number) KB(number * 1000);
#define GB(number) MB(number * 1000);

#ifdef DEBUG
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

struct memory_arena
{
  char *memory;
  unsigned int size;
  unsigned int used;
  void init(void *data, unsigned int s)
  {
    size = s;
    memory = (char *)data;
  }
  void *push(unsigned int amount)
  {
    if ( !(amount > size - used) ) //we can push!
    {
      void *base = memory + used;
      used += amount;
      return base;
    } else
    {
      return NULL; //we return null when there is no more space!
    }
  }
};

#define GAME_INIT(name) void name(engine_memory *memory)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name(engine_memory *memory, user_input *userInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_CLOSE(name) void name()
typedef GAME_CLOSE(game_close);

struct game_code
{
  game_init *GameInit;
  game_update_and_render *GameUpdateAndRender;
  game_close *GameClose;
  unsigned __int64 lastWriteTime;
  bool isValid;
};
