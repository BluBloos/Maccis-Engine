#include <renderer.h>

void Clear()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void DrawNoIndex(game_object object, camera cam)
{
  object.bind();
  cam.bind(object.material.sh);
  glDrawArrays(GL_TRIANGLES, 0, object.mesh.vao.vertexBuffer.size / object.mesh.vao.vertexBuffer.elementSize / 8);
  //NOTE(Noah): There are 8 floats to a vertex
}

void Draw(game_object object, camera cam)
{
  object.bind();
  cam.bind(object.material.sh);

  GL_CALL(glDrawElements(GL_TRIANGLES, object.mesh.indexBuffer.count, GL_UNSIGNED_INT, NULL));

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
