#version 330 core

layout (location = 0) in vec3 aPosition;

uniform mat4 aTransform;

layout (std140) uniform camera
{
	mat4 uViewProjection;
	float uWorldToScreen;
};

void main()
{
	vec4 pos = aTransform * vec4(aPosition.xyz ,1);
	pos = vec4(pos.xyz * uWorldToScreen, 1);
	gl_Position = uViewProjection * pos;
}
