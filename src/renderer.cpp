#include <renderer.h>

void Clear()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Draw(vertex_array vertexArray, index_buffer indexBuffer, material mat)
{
  mat.sh.bind();
  mat.updateUniforms();
  vertexArray.bind();
  indexBuffer.bind();
  glDrawElements(GL_TRIANGLES, indexBuffer.count, GL_UNSIGNED_INT, NULL);
}
