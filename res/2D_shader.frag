#version 330 core

layout(location = 0) out vec4 color;

in vec2 vTexCoord;

uniform sampler2D utexture;

void main()
{
  vec4 texColor = texture(utexture, vTexCoord);

  if(texColor.a < 0.5)
  {
    discard;
  }

  color = texColor;
};
