//zero dependencies

#define MAX_KEY_STATES 26

enum enum_key_state
{
  MACCIS_KEY_W, MACCIS_KEY_A, MACCIS_KEY_S,
  MACCIS_KEY_D, MACCIS_KEY_SPACE, MACCIS_KEY_ESCAPE,
  MACCIS_KEY_SHIFT, MACCIS_KEY_ENTER, MACCIS_KEY_CONTROL,
  MACCIS_KEY_ALT, MACCIS_KEY_LEFT, MACCIS_KEY_RIGHT,
  MACCIS_KEY_UP, MACCIS_KEY_DOWN, MACCIS_MOUSE_LEFT,
  MACCIS_MOUSE_RIGHT, MACCIS_MOUSE_MIDDLE
};

struct key_state
{
  bool endedDown;
  unsigned int halfTransitionCount;
};

struct user_input
{
  key_state keyStates[MAX_KEY_STATES];
  int mouseX, mouseDX, mouseY, mouseDY;
};
