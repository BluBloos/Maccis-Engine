struct text_handle
{
  vec2 
  unsigned int textLength;
  char *text;
  unsigned int fontId;
};

struct profiler
{
  unsigned int registeredTextCount;
  text_handle *registeredTexts;
};

//NOTE(Noah): this function should return null upon failure, which would imply that there is a lack of
//text on the screen
INTERNAL text_handle PushText(profiler *profileHandle, char *text)
{
  //go through all the registered text handle and find space on the screen


  profileHandle->registeredTexts[profileHandle->registeredTextCount++] = newHandle;
}

void UpdateText(text_handle *textHandle, char *text)
{
  textHandle->textLength = GetStringLength(text);
  CloneString(text, textHandle->text, textHandle->textLength);
}
