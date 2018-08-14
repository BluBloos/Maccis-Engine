inline void AddCallTobuffer(renderer *renderer, unsigned char id, unsigned char *param, unsigned int paramSize)
{
  unsigned char *data = renderer->buffer + renderer->currentOffset;
  renderer->currentOffset += paramSize + 1;
  *data++ = id;

  if(param)
  {
    memcpy(data, param, paramSize);
  }
}

inline void Clear(renderer *renderer)
{
  AddCallTobuffer(renderer, 0, NULL, 0);
}

inline void Draw(renderer *renderer, game_object gameObject, camera camera)
{
  AddCallTobuffer(renderer)
}
