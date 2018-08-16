//TODO(Noah): remove dependency on math.h for sin and cos
#include <math.h>

/* dependencies
c standard library (math.h)
*/

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

inline float Min(float a, float b)
{
  if (a < b)
  {
    return a;
  } else
  {
    return b;
  }
}

inline float Max(float a, float b)
{
  if (a > b)
  {
    return a;
  } else
  {
    return b;
  }
}

inline vec2 NewVec2(float x, float y)
{
  vec2 result = {x, y}; return result;
}

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

inline bool PointInBounds(vec2 point, vec2 pos1, vec2 pos2)
{
  float minX = Min(pos1.x, pos2.x);
  float maxX = Max(pos1.x, pos2.x);
  float minY = Min(pos1.y, pos2.y);
  float maxY = Max(pos1.y, pos2.y);

  if (point.x >= minX && point.x <= maxX)
  {
    if (point.y >= minY && point.y <= maxY)
    {
      return true;
    }
  }

  return false;
}

inline float Pow(float b, float e)
{
  return powf(b, e);
}

inline float SquareRoot(float a)
{
  return Pow(a, 1.0f / 2.0f);
}

inline bool CircleLineCollisionTest(vec2 circlePos, float circleRadius, vec2 linePos1, vec2 linePos2, vec2 *posOut)
{
  //determine if the line is vertical
  if (linePos1.x == linePos2.x)
  {
    float beta = linePos1.x;

    //calculate the amount of points of intersection
    float a = 1.0f;
    float b = -2.0f * circlePos.y;
    float c = (beta * beta) - 2.0f * beta * circlePos.x + (circlePos.x * circlePos.x) + (circlePos.y * circlePos.y) - (circleRadius * circleRadius);

    float discriminant = (b * b) - (4.0f * a * c);
    if (discriminant > 0.0f)
    {
      //NOTE(Noah): there are two solutions
      float y1 = (-b + SquareRoot(discriminant)) / (2.0f * a);
      float x1 = beta;

      float y2 = (-b - SquareRoot(discriminant)) / (2.0f * a);
      float x2 = beta;

      bool x1bounds = PointInBounds(NewVec2(x1, y1), linePos1, linePos2);
      bool x2bounds = PointInBounds(NewVec2(x2, y2), linePos1, linePos2);

      if (x1bounds && x2bounds)
      {
        //NOTE(Noah): both are in bounds
        posOut[0] = NewVec2(x1, y1);
        posOut[1] = NewVec2(x2, y2);
        return true;
      } else if(x1bounds)
      {
        //NOTE(Noah): only point1 is in bounds
        posOut[0] = NewVec2(x1, y1);
        return true;
      } else if (x2bounds)
      {
        //NOTE(Noah): only point2 is in bounds
        posOut[0] = NewVec2(x2, y2);
        return true;
      }
    } else if (discriminant == 0.0f)
    {
      //NOTE(Noah): there is one solution
      float y = -b / (2.0f * a);
      float x = beta;

      if (PointInBounds(NewVec2(x, y), linePos1, linePos2))
      {
        posOut[0] = NewVec2(x, y);
        return true;
      }
    }
  } else
  {
    //calucalte line 1
    float gradient = (linePos2.y - linePos1.y) / (linePos2.x - linePos1.x);
    float beta = linePos2.y - gradient * linePos2.x;

    //calculate the amount of points of intersection
    float B = beta - circlePos.y;
    float a = 1.0f + gradient;
    float b = -2.0f * circlePos.x + 2.0f * gradient * B;
    float c = (B * B) + (circlePos.x * circlePos.x) - (circleRadius * circleRadius);

    float discriminant = (b * b) - (4.0f * a * c);
    if (discriminant > 0.0f)
    {
      //NOTE(Noah): there are two solutions
      float x1 = (-b + SquareRoot(discriminant)) / (2.0f * a);
      float y1 = gradient * x1 + beta;

      float x2 = (-b - SquareRoot(discriminant)) / (2.0f * a);
      float y2 = gradient * x2 + beta;

      bool x1bounds = PointInBounds(NewVec2(x1, y1), linePos1, linePos2);
      bool x2bounds = PointInBounds(NewVec2(x2, y2), linePos1, linePos2);

      if (x1bounds && x2bounds)
      {
        //NOTE(Noah): both are in bounds
        posOut[0] = NewVec2(x1, y1);
        posOut[1] = NewVec2(x2, y2);
        return true;
      } else if(x1bounds)
      {
        //NOTE(Noah): only point1 is in bounds
        posOut[0] = NewVec2(x1, y1);
        return true;
      } else if (x2bounds)
      {
        //NOTE(Noah): only point2 is in bounds
        posOut[0] = NewVec2(x2, y2);
        return true;
      }
    } else if (discriminant == 0.0f)
    {
      //NOTE(Noah): there is one solution
      float x = -b / (2.0f * a);
      float y = gradient * x + beta;

      if (PointInBounds(NewVec2(x, y), linePos1, linePos2))
      {
        posOut[0] = NewVec2(x, y);
        return true;
      }
    }
  }
  return false;
}

inline float Absolute(float a)
{
  if(a < 0.0f)
  {
    return -a;
  }
  return a;
}
