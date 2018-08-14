//TODO(Noah): engine.h should be rendering API agnostic
//NOTE(Noah): I think removing the function calls inside each struct can move us closer to an engine who is
//rendering api agnostic

//TODO(Noah): remove dependency on the C standard library
//TODO(Noah): remove dependency on the platform layer

/* dependencies
string.h
maccis_math.h
platform.h
backend.h
*/

#if 0
INTERNAL void GLClearError()
{
  while(glGetError() != GL_NO_ERROR);
}

//TODO(Noah): fix this, it doesn't print
INTERNAL bool GLCheckError(char *function, char *file, int line)
{
  bool result = true;
  while(GLenum error = glGetError())
  {
    //printf("GL_ERROR: %d\nFUNCTION: %s\nFILE: %s\nLINE: %d", error, function, file, line);
    result = false;
  }
  return result;
}

#ifdef DEBUG
#define GL_CALL(code) GLClearError(); code; Assert(GLCheckError(#code, __FILE__, __LINE__));
#else
#define GL_CALL(code) code;
#endif
#endif

struct transform
{
  vec3 position;
  vec3 scl;
  vec3 rotation;

  //NOTE(Noah): These are the three local basic vectors representing orientation for use with local translations!
  vec3 right;
  vec3 up;
  vec3 forward;

  void translate(float dx, float dy, float dz)
  {
    position.x += dx; position.y += dy; position.z += dz;
  }
  void setPosition(float x, float y, float z)
  {
    position.x = x; position.y = y; position.z = z;
  }
  void scale(float x, float y, float z)
  {
    scl.x *= x; scl.y *= y; scl.z *= z;
  }
  void setScale(float x, float y, float z)
  {
    scl.x = x; scl.y = y; scl.z = z;
  }
  void setRotation(float x, float y, float z)
  {
    rotation.x = x; rotation.y = y; rotation.z = z;
  }
  void rotate(float dx, float dy, float dz)
  {
    rotation.x += dx; rotation.y += dy; rotation.z += dz;
  }
};

//TODO(Noah): maybe do some abstraction with this class?
struct camera
{
  //mat4 view;
  mat4 proj;
  transform trans;

  void translateLocal(float dx, float dy, float dz)
  {
    trans.position = AddVec3(trans.position, ScaleVec3(trans.right, -dx) );
    trans.position = AddVec3(trans.position, ScaleVec3(trans.up, -dy) );
    trans.position = AddVec3(trans.position, ScaleVec3(trans.forward, -dz) );
  }

  void translate(float dx, float dy, float dz)
  {
    trans.position.x -= dx; trans.position.y -= dy; trans.position.z -= dz;
  }

  void setPosition(float x, float y, float z)
  {
    trans.position.x = -x; trans.position.y = -y; trans.position.z = -z;
  }

  void setRotation(float x, float y, float z)
  {
    trans.rotation.x = -x; trans.rotation.y = -y; trans.rotation.z = -z;
    //calculate basic vectors based on the rotation
  }

  void rotate(float dx, float dy, float dz)
  {
    trans.rotation.x -= dx; trans.rotation.y -= dy; trans.rotation.z -= dz;

    //create the rotation matrix
    float array[] = {
      cosf(-trans.rotation.y * DEGREES_TO_RADIANS), 0, sinf(-trans.rotation.y * DEGREES_TO_RADIANS), 0,
      0, 1, 0, 0,
      -sinf(-trans.rotation.y * DEGREES_TO_RADIANS), 0, cosf(-trans.rotation.y * DEGREES_TO_RADIANS), 0,
      0, 0, 0, 1
    };

    mat4 rotationMatrixY = {};
    memcpy(rotationMatrixY.matp, array, 16 * sizeof(float));

    trans.forward = TransformVec3Mat4(NewVec3(0.0f, 0.0f, 1.0f), rotationMatrixY);
    trans.right = TransformVec3Mat4(NewVec3(1.0f, 0.0f, 0.0f), rotationMatrixY);

  }
};

//TODO(Noah): maybe rip some stuff out, or refactor?
struct material
{
  shader sh;
  vec4 color;
  texture tex;
  void setColor(float r, float g, float b, float a)
  {
    color.x = r;
    color.y = g;
    color.z = b;
    color.w = a;
  }
  void setTexture(texture t)
  {
    tex = t;
  }
};

struct mesh
{
  vertex_array vao;
  index_buffer indexBuffer;
};

struct game_object
{
  mesh mesh;
  transform transform;
  material material;
};


//NOTE(Noah): do these 2D renderables belong in this header?
struct renderable_2D_vertex
{
  float position[2];
  float textureCoordinate[2];
  float normal[3];
};

struct renderable_2D
{
  vec2 scale;
  vec2 position;
  float rotation;
  unsigned int width;
  unsigned int height;
  float alignPercentage[2];
  renderable_2D_vertex vertices[4];
};

struct batch_renderer_2D
{
  vertex_array vao;
  index_buffer indexBuffer;
  renderable_2D_vertex *vertexBufferMap;
  shader defaultShader;
  texture textureAtlas;
};
