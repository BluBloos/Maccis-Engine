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

struct loaded_bitmap
{
  unsigned int scale;
  unsigned int height;
  unsigned int width;
  unsigned int *pixelPointer;
  void *container;
  void free(platform_free_file FreeFile)
  {
    FreeFile(container);
  }
};

#pragma pack(push, 1)
struct bitmap_header
{
	unsigned short FileType;     /* File type, always 4D42h ("BM") */
	unsigned int FileSize;     /* Size of the file in bytes */
	unsigned short Reserved1;    /* Always 0 */
	unsigned short Reserved2;    /* Always 0 */
	unsigned int BitmapOffset; /* Starting position of image data in bytes */
	unsigned int size;            /* Size of this header in bytes */
	int Width;           /* Image width in pixels */
	int Height;          /* Image height in pixels */
	unsigned short Planes;          /* Number of color planes */
	unsigned short BitsPerPixel;    /* Number of bits per pixel */
};
#pragma pack(pop)
