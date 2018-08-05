#include <windows.h>
#include <stdio.h>

#include <maccis.h>
#include <maccis_strings.h>
#include <platform.h>
#include <maccis_file_io.h>

#include <file_io.cpp>
#include <win32_fonts.cpp>

INTERNAL bool globalRunning = true;

int main()
{
  printf("Welcome to the Maccis-Engine command line interface! Type 'help' for help\n");
  char stringBuffer[256];
  while(globalRunning)
  {
    scanf("%s", stringBuffer);
    if ( StringEquals(stringBuffer, "help") )
    {

    }
    else if ( StringEquals(stringBuffer, "font") )
    {
      printf("Enter a font name\n");
      scanf("%s", stringBuffer);
      //build the font asset
    } else
    {
      printf("Invalid command!\n");
    }
  }
}
