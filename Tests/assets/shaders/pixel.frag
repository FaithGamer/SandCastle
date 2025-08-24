#version 330 core

out vec4 oColor;

in vec4  vColor;
in vec2  vTexCoords;
in float vTexIndex;

uniform sampler2D uTextures[16];

// Sample at texel centers and quantize gradients so each texel
// occupies a constant number of screen pixels (pixel-perfect).
vec4 samplePixelPerfect(sampler2D tex, vec2 uv)
{
    vec2 dim      = vec2(textureSize(tex, 0));   // texture size in texels
    vec2 uvTexel  = uv * dim;                    // convert to texel space
    vec2 uvCenter = (floor(uvTexel) + 0.5) / dim; // snap to texel centers

    // Gradients in texel units, then quantize to whole texels per pixel
    vec2 gdx = dFdx(uv * dim);
    vec2 gdy = dFdy(uv * dim);
    gdx = round(gdx) / dim;
    gdy = round(gdy) / dim;

    return textureGrad(tex, uvCenter, gdx, gdy);
}

void main()
{
    int idx = int(vTexIndex + 0.5);

    // Select texture and sample pixel-perfect
    switch (idx)
    {
        case 0:  oColor = samplePixelPerfect(uTextures[0],  vTexCoords) * vColor; break;
        case 1:  oColor = samplePixelPerfect(uTextures[1],  vTexCoords) * vColor; break;
        case 2:  oColor = samplePixelPerfect(uTextures[2],  vTexCoords) * vColor; break;
        case 3:  oColor = samplePixelPerfect(uTextures[3],  vTexCoords) * vColor; break;
        case 4:  oColor = samplePixelPerfect(uTextures[4],  vTexCoords) * vColor; break;
        case 5:  oColor = samplePixelPerfect(uTextures[5],  vTexCoords) * vColor; break;
        case 6:  oColor = samplePixelPerfect(uTextures[6],  vTexCoords) * vColor; break;
        case 7:  oColor = samplePixelPerfect(uTextures[7],  vTexCoords) * vColor; break;
        case 8:  oColor = samplePixelPerfect(uTextures[8],  vTexCoords) * vColor; break;
        case 9:  oColor = samplePixelPerfect(uTextures[9],  vTexCoords) * vColor; break;
        case 10: oColor = samplePixelPerfect(uTextures[10], vTexCoords) * vColor; break;
        case 11: oColor = samplePixelPerfect(uTextures[11], vTexCoords) * vColor; break;
        case 12: oColor = samplePixelPerfect(uTextures[12], vTexCoords) * vColor; break;
        case 13: oColor = samplePixelPerfect(uTextures[13], vTexCoords) * vColor; break;
        case 14: oColor = samplePixelPerfect(uTextures[14], vTexCoords) * vColor; break;
        case 15: oColor = samplePixelPerfect(uTextures[15], vTexCoords) * vColor; break;
        default: oColor = vec4(0.0); break;
    }

    if (oColor.a < 0.2) discard;
}
