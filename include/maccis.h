#define INTERNAL static
#define PERSISTENT static
#define MAX_KEY_STATES 26

#define KB(number) number * 1000;
#define MB(number) KB(number * 1000);
#define GB(number) MB(number * 1000);

#ifdef DEBUG
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

INTERNAL void GLClearError()
{
  while(glGetError() != GL_NO_ERROR);
}

INTERNAL bool GLCheckError(char *function, char *file, int line)
{
  bool result = true;
  while(GLenum error = glGetError())
  {
    printf("GL_ERROR: %d\nFUNCTION: %s\nFILE: %s\nLINE: %d", error, function, file, line);
    result = false;
  }
  return result;
}

#ifdef DEBUG
#define GL_CALL(code) GLClearError(); code; Assert(GLCheckError(#code, __FILE__, __LINE__));
#else
#define GL_CALL(code) code;
#endif

enum enum_key_state
{
  MACCIS_KEY_W, MACCIS_KEY_A, MACCIS_KEY_S,
  MACCIS_KEY_D, MACCIS_KEY_SPACE, MACCIS_KEY_ESCAPE,
  MACCIS_KEY_SHIFT, MACCIS_KEY_ENTER, MACCIS_KEY_CONTROL,
  MACCIS_KEY_ALT, MACCIS_KEY_LEFT, MACCIS_KEY_RIGHT,
  MACCIS_KEY_UP, MACCIS_KEY_DOWN
};

struct key_state
{
  bool endedDown;
  unsigned int halfTransitionCount;
};

struct user_input
{
  key_state keyStates[MAX_KEY_STATES];
};

struct read_file_result
{
  void *content;
  unsigned int contentSize;
};

struct file_path
{
  char path[260];
  unsigned int length;
};

struct memory_arena
{
  void *memory;
  unsigned int size;
  unsigned int used;
  void init(void *data, unsigned int s)
  {
    size = s;
    memory = data;
  }
  void *push(unsigned int amount)
  {
    if ( !(amount > size - used) ) //we can push!
    {
      void *base = (char *)memory + used;
      used += amount;
      return base;
    } else
    {
      return NULL; //we return null when there is no more space!
    }
  }
};

#include <platform.h>
#include <maccis_io.h>
#include <maccis_strings.h>
#include <io.cpp>
