#version 330 core

out vec4 oColor;
in vec2 vTexCoords;

uniform sampler2D uTextures[16];

void main()
{
	vec4 colorMask = texture(uTextures[1], vTexCoords);

	vec4 color = texture(uTextures[0], vTexCoords);

	//color.a = min(color.a, colorMask.a); positive mask
	color.a -= colorMask.a;
	
	oColor = color;
}