
#ifdef NULL
#else
#define NULL 0
#endif

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

enum enum_type
{
  MACCIS_FLOAT,
  MACCIS_DOUBLE,
  MACCIS_UINT, MACCIS_INT,
  MACCIS_SHORT, MACCIS_USHORT,
  MACCIS_CHAR, MACCIS_UCHAR
};

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
