#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in float aTexIndex;

out vec2 vTexCoords;
out float vTexIndex;

uniform mat4 projectionView;
uniform mat4 model;

void main()
{
	vTexIndex = aTexIndex;
	vTexCoords = aTexCoords;
	gl_Position = projectionView * model * vec4(aPos, 1.0);
}