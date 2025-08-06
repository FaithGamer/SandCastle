#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in float aTexIndex;

out vec4 vColor;
out vec2 vTexCoords;
out float vTexIndex;

layout (std140) uniform camera
{
	mat4 uViewProjection;
	float uWorldScreenRatio;
};

void main()
{
	vColor = aColor;
	vTexCoords = aTexCoords;
	vTexIndex = aTexIndex;
	vec4 pos = vec4(aPosition.xyz * uWorldScreenRatio, 1);
	gl_Position = uViewProjection * pos;
}