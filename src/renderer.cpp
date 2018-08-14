//NOTE(Noah): Currently the renderer depends on openGL. Should the renderer be aware of the API or be API agnostic?
//I think it's essential that the renderer is API specific, since Vulkan is so dramatically different than openGL.

/*dependencies
gl.h
engine.h
standard c library
maccis_file_io.h
maccis_strings.h
engine_utilities.cpp
maccis_math.h
*/

void Clear()
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    material.sh.setUniformMat4f("umodel", t.buildMatrix());
    glDrawElements(GL_TRIANGLES, mesh.indexBuffer.count, GL_UNSIGNED_INT, NULL);
  }
}

#define RENDERER_MAX_SPRITES 10000
#define RENDERER_VERTEX_SIZE 7 * sizeof(float)
#define RENDERER_BUFFER_SIZE RENDERER_MAX_SPRITES * 4 * RENDERER_VERTEX_SIZE
#define RENDERER_INDICES_SIZE RENDERER_MAX_SPRITES * 6

//2D batch rendering
void InitializeBatchRenderer2D(batch_renderer_2D *batchRenderer2D, shader defaultShader)
{
  batchRenderer2D->defaultShader = defaultShader;
  batchRenderer2D->vao = CreateVertexArray();

  //NOTE(Noah): it costs alot to constantly bind and unbind
  //make a vertex buffer
  vertex_buffer vertexBuffer = {};
  vertexBuffer.elementSize = sizeof(float);
  vertexBuffer.size = RENDERER_BUFFER_SIZE;
  glGenBuffers(1, &vertexBuffer.id);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.id);
  glBufferData(GL_ARRAY_BUFFER, RENDERER_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

  //setup the buffer layout desribing the contents of a vertex
  buffer_layout bufferLayout = CreateBufferLayout();
  bufferLayout.push(2, GL_FLOAT); //position
  bufferLayout.push(2, GL_FLOAT); //texture coordinates
  bufferLayout.push(3, GL_FLOAT); //normals
  batchRenderer2D->vao.addBuffer(vertexBuffer, bufferLayout);

  //unbind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //generate the index buffer
  unsigned int indices[RENDERER_INDICES_SIZE];
  unsigned int offset = 0;
  for (unsigned int i = 0; i < RENDERER_INDICES_SIZE; i += 6)
  {
    indices[  i  ] = offset + 0;
    indices[i + 1] = offset + 1;
    indices[i + 2] = offset + 2;
    indices[i + 3] = offset + 2;
    indices[i + 4] = offset + 3;
    indices[i + 5] = offset + 0;
    offset += 4;
  }

  //make an index buffer on gpu
  batchRenderer2D->indexBuffer = CreateIndexBuffer(indices, RENDERER_INDICES_SIZE);
  batchRenderer2D->indexBuffer.count = 0;
}

void BeginBatchRenderer2D(batch_renderer_2D *batchRenderer2D)
{
  batchRenderer2D->vao.bind();
  batchRenderer2D->vao.vertexBuffer.bind();
  batchRenderer2D->vertexBufferMap = (renderable_2D_vertex *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

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

void EndBatchRenderer2D(batch_renderer_2D *batchRenderer2D)
{
  glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Flush(batch_renderer_2D *batchRenderer2D, camera cam)
{
  batchRenderer2D->defaultShader.bind();
  cam.bind(batchRenderer2D->defaultShader);
  batchRenderer2D->indexBuffer.bind();
  batchRenderer2D->textureAtlas.bind();
  batchRenderer2D->defaultShader.setUniform1i("utexture", batchRenderer2D->textureAtlas.slot);

  glDrawElements(GL_TRIANGLES, batchRenderer2D->indexBuffer.count, GL_UNSIGNED_INT, 0);

  batchRenderer2D->vao.unbind();
  batchRenderer2D->vao.vertexBuffer.unbind();
  batchRenderer2D->indexBuffer.unbind();
  batchRenderer2D->indexBuffer.count = 0;
}

void DebugPushText(char *string, batch_renderer_2D *batchRenderer2D, loaded_font *font)
{
  renderable_2D *fontSprites = (renderable_2D *)font->fontSprites;
  unsigned int stringLength = GetStringLength(string);
  float xOffset = 100.0f;
  float baseline = 100.0f;
  char *scan = string;
  char prevCharacter = *scan;
  for (unsigned int i = 0; i < stringLength; i++)
  {
    char character = *scan;

    if (prevCharacter != 0)
    {
      xOffset += GetHorizontalAdvanceForPair(font, prevCharacter, character);
    }

    renderable_2D sprite = fontSprites[GetIndexFromCharacter(font, character)];
    sprite.position = NewVec2(xOffset + sprite.width * sprite.alignPercentage[0], baseline - sprite.height * sprite.alignPercentage[1]);
    Submit(batchRenderer2D, sprite);
      //xOffset += sprite.width + 1.0f;

    prevCharacter = *scan;
    scan++;
  }

  //special case space
  //do proper verical alignment to the baseline via getting the text metrics
}
