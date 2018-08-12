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
  void *vertices;
  void *indices;
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
  unsigned int biCompression; //specfies type of compression to be used
  unsigned int biSizeImage; //can be set to 0 for uncompressed RGB bitmaps
  int biXPelsPerMeter; //horizontal resolution in pixels per meter
  int biYPelsPerMeter; //vertical resoltion in pixels per meter
  unsigned int biClrUsed; //specifies color indices used in color table
  unsigned int biClrImportant; //specifies color indices that are important
  char rgbBlue;
  char rgbGreen;
  char rgbRed;
  char rgbReserved;
};
#pragma pack(pop)

struct character_desriptor
{
  float textureCoordinate[8]; //a character has 8 texture coordinates into the atlas
  unsigned int width;
  unsigned int height;
  float alignPercentage[2];
  //ascenders and descenders
};

struct loaded_font
{
  //NOTE(Noah): These are dynamically allocated since different fonts have different
  //amounts of glyphs
  unsigned int codePointCount;
  unsigned int firstChar;
  unsigned int lastChar;
  void *fontSprites;
  float lineHeight;
  float *horizontalAdvance;
};
