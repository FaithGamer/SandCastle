#version 330 core

uniform sampler2D texture1;
uniform sampler2D texture2;


in vec2 vTexCoords;
in float vTexIndex;

out vec4 oColor;

void main()
{
oColor = vec4(1.0, 0.0, 0.0, 1.0);
	switch(int(round(vTexIndex)))
	{	
	case 0:
		oColor = texture(texture1, vTexCoords);
		break;
	case 1:
		oColor = texture(texture2, vTexCoords);
		break;
	default:
		oColor = vec4(1.0, 0.0, 0.0, 1.0);
		break;
	}

}