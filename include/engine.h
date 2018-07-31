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
  vertex_array vertexArray;
  index_buffer indexBuffer;
  material defaultMaterial;
  texture defaultTexture;
  char r;
};
