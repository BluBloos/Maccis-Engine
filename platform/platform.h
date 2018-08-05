struct read_file_result
{
  void *content;
  unsigned int contentSize;
};

#define PLATFORM_READ_FILE(name) read_file_result name(char *filePath)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_FREE_FILE(name) void name(void *memory)
typedef PLATFORM_FREE_FILE(platform_free_file);
