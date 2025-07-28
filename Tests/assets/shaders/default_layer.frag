#version 330 core

out vec4 oColor;
in vec2 vTexCoords;

uniform sampler2D uTextures[16];

void main()
{
	vec4 color = texture(uTextures[0], vTexCoords);
	oColor = color;
}