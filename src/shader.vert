#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 vTexCoord;

uniform mat4 uproj;
uniform mat4 umodel;
uniform mat4 uview;

void main()
{
  gl_Position = uproj * uview * umodel * vec4(position.x, position.y, position.z, 1);
  vTexCoord = texCoord;
};
