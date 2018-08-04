#include <math.h>

#define PI 3.1415f
#define DEGREES_TO_RADIANS PI / 180.0f

struct vec2
{
  float x,y;
};

struct vec3
{
  float x,y,z;
};

struct vec4
{
  float x,y,z,w;
};

struct mat4
{
  union
  {
    float mat[4][4];
    float matp[16];
    vec4 matv[4];
  };
};

inline vec3 NewVec3(float x, float y, float z)
{
  vec3 result = {x, y, z}; return result;
}

//TODO(Noah): Make all math operations super fast, since otherwise our engine is shit!
inline vec3 AddVec3(vec3 a, vec3 b)
{
  vec3 c = {};
  c.x = a.x + b.x;
  c.y = a.y + b.y;
  c.z = a.z + b.z;
  return c;
}

inline vec3 ScaleVec3(vec3 a, float scalar)
{
  vec3 c = {};
  c.x = a.x * scalar;
  c.y = a.y * scalar;
  c.z = a.z * scalar;
  return c;
}

inline vec3 TransformVec3Mat4(vec3 a, mat4 b)
{
  vec3 c = {};
  c.x = a.x * b.mat[0][0] + a.y * b.mat[1][0] + a.z * b.mat[2][0]; 
  c.y = a.x * b.mat[0][1] + a.y * b.mat[1][1] + a.z * b.mat[2][1];
  c.z = a.x * b.mat[0][2] + a.y * b.mat[1][2] + a.z * b.mat[2][2];
  return c;
}

inline vec4 TransformVec4(vec4 a, mat4 b)
{
  vec4 c = {};
  c.x = a.x * b.mat[0][0] + a.y * b.mat[1][0] + a.z * b.mat[2][0] + a.w * b.mat[3][0];
  c.y = a.x * b.mat[0][1] + a.y * b.mat[1][1] + a.z * b.mat[2][1] + a.w * b.mat[3][1];
  c.z = a.x * b.mat[0][2] + a.y * b.mat[1][2] + a.z * b.mat[2][2] + a.w * b.mat[3][2];
  c.w = a.x * b.mat[0][3] + a.y * b.mat[1][3] + a.z * b.mat[2][3] + a.w * b.mat[3][3];
  return c;
}

inline mat4 TransformMatrix(mat4 a, mat4 b)
{
  mat4 c = {};
  c.matv[0] = TransformVec4(a.matv[0], b);
  c.matv[1] = TransformVec4(a.matv[1], b);
  c.matv[2] = TransformVec4(a.matv[2], b);
  c.matv[3] = TransformVec4(a.matv[3], b);
  return c;
}
