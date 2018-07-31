#include <stdio.h>

extern "C" void Print(char *message)
{
  printf("%s\n", message);
}
