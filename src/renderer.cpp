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
  glDrawElements(GL_TRIANGLES, object.mesh.indexBuffer.count, GL_UNSIGNED_INT, NULL);
}
void DrawBatch(material material, mesh mesh, camera cam, transform *objects, unsigned int count)
{
  material.sh.bind();
  material.updateUniforms();
  mesh.bind();
  cam.bind(material.sh);
  for (unsigned int i = 0; i < count; i++)
  {
    transform t = objects[i];
    t.bindMatrix(material.sh);
    glDrawElements(GL_TRIANGLES, mesh.indexBuffer.count, GL_UNSIGNED_INT, NULL);
  }
}
