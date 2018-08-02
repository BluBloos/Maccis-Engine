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

struct raw_model
{
  void *mem;
  unsigned int indicesOffset;
  unsigned int vertexCount;
  unsigned int indexCount;
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
