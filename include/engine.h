struct engine_memory
{
  void *storage;
  long int storageSize;
  file_path maccisDirectory;
  platform_read_file *ReadFile;
  platform_free_file *FreeFile;
};

struct engine_state
{
  texture defaultTexture;
  game_object defaultObject;
  camera mainCamera;
  char r;
};
