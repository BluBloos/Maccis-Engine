//TODO(Noah): remove dependency on the standar C library

/* dependencies
platform.h
maccis_file_io.h
maccis.h
c library
*/

void SaveBitmap(char *fileName, platform_write_file *WriteFile, loaded_bitmap bitmap, memory_arena *arena)
{
	bitmap_header header = {};
	header.FileType = 0x4D42; /* File type, always 4D42 ("BM") */
	header.FileSize = sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int); /* Size of the file in bytes */
	header.BitmapOffset = sizeof(bitmap_header); /* Starting position of image data in bytes */
	header.size = 40; /* Size of this header in bytes */
	header.Width = bitmap.width; /* Image width in pixels */
	header.Height = bitmap.height; /* Image height in pixels */
	header.Planes = 1; /* Number of color planes, must be set to 1 */
	header.BitsPerPixel = 32; /* Number of bits per pixel */
  header.biCompression = BI_RGB; //specfies type of compression to be used

	void *data = arena->push(sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int));
	char *scan = (char *)data;
	memcpy(scan, &header, sizeof(bitmap_header));
	scan += sizeof(bitmap_header);
	memcpy(scan, bitmap.pixelPointer, bitmap.width * bitmap.height * sizeof(unsigned int));
	WriteFile(fileName, sizeof(bitmap_header) + bitmap.width * bitmap.height * sizeof(unsigned int), data);
}

inline loaded_bitmap MakeEmptyBitmap(memory_arena *arena, unsigned int width, unsigned int height)
{
	loaded_bitmap bitmap;
	bitmap.scale = 1;
	bitmap.width = width;
	bitmap.height = height;
	bitmap.pixelPointer = (unsigned int *)arena->push(width * height * sizeof(unsigned int));
	return bitmap;
}

loaded_bitmap LoadBMP(platform_read_file *ReadFile, char *path)
{
	loaded_bitmap bitmap = {};
	bitmap.scale = 1;
	read_file_result fileResult = ReadFile(path);
	if (fileResult.contentSize != 0)
	{
		bitmap_header *header = (bitmap_header *)fileResult.content;
		bitmap.pixelPointer = (unsigned int *) ( (unsigned char *)fileResult.content + header->BitmapOffset );
		bitmap.height = header->Height;
		bitmap.width = header->Width;
    bitmap.container = fileResult.content;
	}
	return bitmap;
}
