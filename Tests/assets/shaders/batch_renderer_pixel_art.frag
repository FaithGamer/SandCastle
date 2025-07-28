#version 330 core

out vec4 oColor;

in vec4 vColor;
in vec2 vTexCoords;
in float vTexIndex;

uniform sampler2D uTextures[16];
uniform float pixelPerTexel;

void main()
{
	vec2 texSize = textureSize(uTextures[int(round(vTexIndex))], 0);

	vec2 tx = vTexCoords * texSize.xy;
	vec2 txOffset = clamp(fract(tx) * pixelPerTexel, 0, 0.5) - clamp((1 - fract(tx)) * pixelPerTexel, 0, 0.5);

	vec2 uv = (floor(tx) + 0.5 + txOffset) / texSize.xy;

	vec4 color = texture(uTextures[int(round(vTexIndex))], uv) * vColor;

	if(color.a <0.01)
		discard;

	oColor = color;
}