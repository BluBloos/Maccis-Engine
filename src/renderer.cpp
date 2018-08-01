#include <renderer.h>

void Clear()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Draw(game_object object, camera cam)
{
  object.bind();
  cam.bind(object.material.sh);
  glDrawElements(GL_TRIANGLES, object.indexBuffer.count, GL_UNSIGNED_INT, NULL);
}
