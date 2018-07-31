#include <stdio.h>
#include <fcntl.h>
#include <io.h>

// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void Win32OpenConsole()
{
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

  // allocate a console for this app
  //https://docs.microsoft.com/en-us/windows/console/allocconsole
  AllocConsole();

  // set the screen buffer to be big enough to let us scroll text
  //https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  // redirect unbuffered STDOUT to the console
  //https://msdn.microsoft.com/en-us/library/bdts1c9x.aspx
  //https://msdn.microsoft.com/en-us/library/88k7d7a7.aspx
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  *stdout = *fp;
  //setvbuf( stdout, NULL, _IONBF, 0 ); //associate no buffer
  freopen_s( &fp, "CONOUT$", "w", stdout);
}
