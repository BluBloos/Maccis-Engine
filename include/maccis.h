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
