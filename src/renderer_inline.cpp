/* dependencies
engine.h
string.h
*/

inline void Submit(batch_renderer_2D *batchRenderer2D, renderable_2D renderable)
{
  renderable.vertices[0].position[0] += renderable.position.x;
  renderable.vertices[0].position[1] += renderable.position.y;

  renderable.vertices[1].position[0] += renderable.position.x;
  renderable.vertices[1].position[1] += renderable.position.y;

  renderable.vertices[2].position[0] += renderable.position.x;
  renderable.vertices[2].position[1] += renderable.position.y;

  renderable.vertices[3].position[0] += renderable.position.x;
  renderable.vertices[3].position[1] += renderable.position.y;

  memcpy(batchRenderer2D->vertexBufferMap, renderable.vertices,
    sizeof(renderable_2D_vertex) * 4);
  batchRenderer2D->vertexBufferMap += 4;
  batchRenderer2D->indexBuffer.count += 6;
}
