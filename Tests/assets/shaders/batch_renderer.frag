#version 330 core

out vec4 oColor;

in vec4 vColor;
in vec2 vTexCoords;
in float vTexIndex;

uniform sampler2D uTextures[16];

void main()
{
vec2 uvDx = dFdx(vTexCoords);
vec2 uvDy = dFdy(vTexCoords);

switch(int(round(vTexIndex)))
{
case 0:
oColor = textureGrad(uTextures[0], vTexCoords, uvDx, uvDy) * vColor;
break;
case 1:
oColor = textureGrad(uTextures[1], vTexCoords, uvDx, uvDy) * vColor;
break;
case 2:
oColor = textureGrad(uTextures[2], vTexCoords, uvDx, uvDy) * vColor;
break;
case 3:
oColor = textureGrad(uTextures[3], vTexCoords, uvDx, uvDy) * vColor;
break;
case 4:
oColor = textureGrad(uTextures[4], vTexCoords, uvDx, uvDy) * vColor;
break;
case 5:
oColor = textureGrad(uTextures[5], vTexCoords, uvDx, uvDy) * vColor;
break;
case 6:
oColor = textureGrad(uTextures[6], vTexCoords, uvDx, uvDy) * vColor;
break;
case 7:
oColor = textureGrad(uTextures[7], vTexCoords, uvDx, uvDy) * vColor;
break;
case 8:
oColor = textureGrad(uTextures[8], vTexCoords, uvDx, uvDy) * vColor;
break;
case 9:
oColor = textureGrad(uTextures[9], vTexCoords, uvDx, uvDy) * vColor;
break;
case 10:
oColor = textureGrad(uTextures[10], vTexCoords, uvDx, uvDy) * vColor;
break;
case 11:
oColor = textureGrad(uTextures[11], vTexCoords, uvDx, uvDy) * vColor;
break;
case 12:
oColor = textureGrad(uTextures[12], vTexCoords, uvDx, uvDy) * vColor;
break;
case 13:
oColor = textureGrad(uTextures[13], vTexCoords, uvDx, uvDy) * vColor;
break;
case 14:
oColor = textureGrad(uTextures[14], vTexCoords, uvDx, uvDy) * vColor;
break;
case 15:
oColor = textureGrad(uTextures[15], vTexCoords, uvDx, uvDy) * vColor;
break;

}

	if(oColor.a <0.2)
		discard;

}