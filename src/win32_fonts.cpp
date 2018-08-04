#define STB_TRUETYPE_IMPLEMENTATION
#include <vendor/stb_truetype.h>

INTERNAL loaded_bitmap MakeNothingsTest(platform_read_file *ReadFile, memory_arena arena)
{
  loaded_bitmap result = {};
  //NOTE(Noah): It's okay that this path is absolute since all fonts are stored in the same locations
  //on all installations of windows (I hope).
  read_file_result fileResult = ReadFile("C:\\Windows\\Fonts\\arial.ttf");
  stbtt_fontinfo font;

  /* Given an offset into the file that defines a font, this function builds
     the necessary cached info for the rest of the system. You must allocate
     the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
     need to do anything special to free it, because the contents are pure
     value data with no additional data structures. Returns 0 on failure. */
  // STBTT_DEF int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset);

  // Each .ttf/.ttc file may have more than one font. Each font has a sequential
  // index number starting from 0. Call this function to get the font offset for
  // a given index; it returns -1 if the index is out of range. A regular .ttf
  // file will only define one font and it always be at offset 0, so it will
  // return '0' for index 0, and -1 for all other indices.
  // STBTT_DEF int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index);

  // STBTT_DEF unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info,
  // float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
  // allocates a large-enough single-channel 8bpp bitmap and renders the
  // specified character/glyph at the specified scale into it, with
  // antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
  // *width & *height are filled out with the width & height of the bitmap,
  // which is stored left-to-right, top-to-bottom.
  // xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

  // STBTT_DEF float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float pixels);
  // computes a scale factor to produce a font whose "height" is 'pixels' tall.
  // Height is measured as the distance from the highest ascender to the lowest
  // descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
  // and computing:
  //       scale = pixels / (ascent - descent)
  // so if you prefer to measure height by the ascent only, use a similar calculation.

  if(!stbtt_InitFont(&font, (unsigned char *)fileResult.content, stbtt_GetFontOffsetForIndex((unsigned char *)fileResult.content, 0)))
  {
    //TODO(Noah): logging since it failed to initialize the font
  }

  int width, height, xoffset, yoffset;
  unsigned char *monoBitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 120.0f), 'N',
    &width, &height, &xoffset, &yoffset);

  //Allocate a bitmap so we can fill it!
  result = MakeEmptyBitmap(arena, width, height);

  unsigned char *source = monoBitmap;
  unsigned int *destRow = result.pixelPointer;
  for (unsigned int y = 0; y < height; y++)
  {
    unsigned int *dest = (unsigned int *)destRow;
    for (unsigned int x = 0; x < width; x++)
    {
      unsigned char alpha = *source++;
      *dest++ = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    }
    destRow += width;
  }

  //This function will free an 8bpp bitmap
  stbtt_FreeBitmap(monoBitmap, 0);

  //finally return the bitmap which was so painstakingly crafted!
  return result;
}
