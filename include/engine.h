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
  memory_arena memoryArena;
  texture defaultTexture;
  texture testTexture;
  game_object defaultObject;
  game_object suzanne;
  camera mainCamera;
  transform dummyObjects[10];
};
