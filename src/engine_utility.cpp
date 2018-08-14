/* dependency
engine.h
maccis_file_io.h
*/

//TODO(Noah): fix this? or remove it. Why is it an engine engine utility.
void DrawBitmapUnchecked(loaded_bitmap bitmap, unsigned int *pixelPointer, unsigned int pixelPitch)
{
  unsigned int *destRow = pixelPointer;
  unsigned int *sourceRow = bitmap.pixelPointer;
  for (unsigned int y = 0; y < bitmap.height; y++)
  {
    unsigned int *source = sourceRow;
    unsigned int *dest = destRow;
    for (unsigned int x =0; x < bitmap.width; x++)
    {
      *dest++ = *source++;
    }
    sourceRow += bitmap.width;
    destRow += pixelPitch;
  }
}

//TODO(Noah): do these belong here?
unsigned int GetIndexFromCharacter(loaded_font *font, unsigned int character)
{
  return character - font->firstChar;
}

float GetHorizontalAdvanceForPair(loaded_font *font, unsigned char character1, unsigned char character2)
{
  //do stuff here
  float result = font->horizontalAdvance[character1 * font->codePointCount + character2];
  return result;
}

inline mat4 BuildMatrixFromTransform(transform *transform, bool translateFirst)
{
  float array[] = {
    transform->scl.x,0,0,0,
    0,transform->scl.y,0,0,
    0,0,transform->scl.z,0,
    0,0,0,1
  };

  mat4 bufferMatrix = {};
  memcpy(bufferMatrix.matp, array, 16 * sizeof(float));

  if (translateFirst)
  {
    bufferMatrix.mat[3][0] = transform->position.x;
    bufferMatrix.mat[3][1] = transform->position.y;
    bufferMatrix.mat[3][2] = transform->position.z;
  }

  float array2[] = {
    cosf(transform->rotation.y * DEGREES_TO_RADIANS), 0, sinf(transform->rotation.y * DEGREES_TO_RADIANS), 0,
    0, 1, 0, 0,
    -sinf(transform->rotation.y * DEGREES_TO_RADIANS), 0, cosf(transform->rotation.y * DEGREES_TO_RADIANS), 0,
    0, 0, 0, 1
  };

  mat4 rotationMatrixY = {};
  memcpy(rotationMatrixY.matp, array2, 16 * sizeof(float));

  bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixY);

  float array3[] = {
    1, 0, 0, 0,
    0, cosf(transform->rotation.x * DEGREES_TO_RADIANS), sinf(transform->rotation.x * DEGREES_TO_RADIANS), 0,
    0, -sinf(transform->rotation.x * DEGREES_TO_RADIANS), cosf(transform->rotation.x * DEGREES_TO_RADIANS), 0,
    0, 0, 0, 1
  };

  mat4 rotationMatrixZ = {};
  memcpy(rotationMatrixZ.matp, array3, 16 * sizeof(float));

  bufferMatrix = TransformMatrix(bufferMatrix, rotationMatrixZ);

  if(!translateFirst)
  {
    bufferMatrix.mat[3][0] = transform->position.x;
    bufferMatrix.mat[3][1] = transform->position.y;
    bufferMatrix.mat[3][2] = transform->position.z;
  }

  return bufferMatrix;
}

inline void BindCamera(camera *camera, shader *shader)
{
  SetUniformMat4f(shader, "uproj", camera->proj);
  SetUniformMat4f(shader, "uview", BuildMatrixFromTransform(&camera->trans, true));
}

inline void BindTextureToShader(texture *texture, shader *shader)
{
  SetUniform1i(shader, "utexture", texture->slot);
}

inline void BindMesh(mesh *mesh)
{
    BindVertexArray(&mesh->vao);
    BindVertexBuffer(&mesh->vao.vertexBuffer);
    BindIndexBuffer(&mesh->indexBuffer);
}

inline void UpdateMaterialUniforms(material *material)
{
  SetUniform4f(&material->sh, "ucolor", material->color.x, material->color.y, material->color.z, material->color.w);
  SetUniform1i(&material->sh, "utexture", material->tex.slot);
}

inline void BindGameObject(game_object *gameObject)
{
  BindShader(&gameObject->material.sh);
  BindTexture(&gameObject->material.tex);
  UpdateMaterialUniforms(&gameObject->material);
  SetUniformMat4f(&gameObject->material.sh, "umodel", BuildMatrixFromTransform(&gameObject->transform, false));
  BindMesh(&gameObject->mesh);
}

#if 0
game_object GameObjectFromRawModel(raw_model model, shader sh)
{
  game_object gameObject = {};

  gameObject.mesh.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer((float *)model.vertices, model.vertexCount * 8); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout

  PushToBufferLayout(&bufferLayout, 3, MACCIS_FLOAT);
  PushToBufferLayout(&bufferLayout, 2, MACCIS_FLOAT);
  PushToBufferLayout(&bufferLayout, 3, MACCIS_FLOAT);
  AddBufferToVertexArray(&gameObject.mesh.vao, &vertexBuffer, &bufferLayout);

  gameObject.mesh.indexBuffer = CreateIndexBuffer((unsigned int *)model.indices, model.indexCount);

  gameObject.material.sh = sh;
  gameObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gameObject.transform.setScale(1.0f, 1.0f, 1.0f);
  gameObject.transform.setPosition(0.0f, 0.0f, -1.0f);

  return gameObject;
}
#endif

game_object GameObjectFromRawModel(raw_model model, shader shader, texture texture)
{
  game_object gameObject = {};

  gameObject.mesh.vao = CreateVertexArray(); //make the vao
  vertex_buffer vertexBuffer = CreateVertexBuffer((float *)model.vertices, model.vertexCount * 8); //make the vertex buffer
  buffer_layout bufferLayout = CreateBufferLayout(); //make a buffer layout

  PushToBufferLayout(&bufferLayout, 3, MACCIS_FLOAT);
  PushToBufferLayout(&bufferLayout, 2, MACCIS_FLOAT);
  PushToBufferLayout(&bufferLayout, 3, MACCIS_FLOAT);
  AddBufferToVertexArray(&gameObject.mesh.vao, &vertexBuffer, &bufferLayout);

  gameObject.mesh.indexBuffer = CreateIndexBuffer((unsigned int *)model.indices, model.indexCount);

  gameObject.material.sh = shader;
  gameObject.material.setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gameObject.material.setTexture(texture);
  gameObject.transform.setScale(1.0f, 1.0f, 1.0f);
  gameObject.transform.setPosition(0.0f, 0.0f, -1.0f);

  return gameObject;
}
