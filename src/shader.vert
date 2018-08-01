#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;

uniform mat4 uproj;
uniform mat4 umodel;

void main()
{
  gl_Position = uproj * umodel * vec4(position.x, position.y, 0, 1);
  vTexCoord = texCoord;
};
