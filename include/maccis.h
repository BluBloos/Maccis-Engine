#define INTERNAL static
#define PERSISTENT static

#ifdef DEBUG
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
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

#include <platform.h>
