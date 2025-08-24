#version 330 core

//Vertex
layout(location = 0) in vec3 iVertexPos;
layout(location = 1) in vec2 iUv;      
layout(location = 2) in vec4 iColor;      
layout(location = 3) in float iTexIndex;       


out vec2 vTexCoords;
out vec4 vColor;
out float vTexIndex;

layout(std140) uniform scene
{
    mat4 uCamProj;
    float uCamZoom;
    float uCamAspectRatio;
    float uWinHeight;
};


void main()
{
    vColor     = iColor;
    vTexCoords = iUv;
    vTexIndex  = iTexIndex;

    // World -> clip
    vec3 pos     = iVertexPos * uCamZoom;
    vec4 clipPos = uCamProj * vec4(pos, 1.0);

    // --- Pixel-perfect snap in screen space (to pixel CENTERS) ---
    // Convert to NDC
    vec2 ndc = clipPos.xy / clipPos.w;

    // Framebuffer size in *pixels* (actual GL viewport size)
    vec2 fbSize = vec2(uWinHeight * uWinHeight * uCamAspectRatio, uWinHeight);

    // Snap to nearest pixel center:
    // pixel center NDC = ((i + 0.5) * 2 / size) - 1
    // so we invert that mapping, floor to whole pixels, add 0.5 (center), and map back.
    vec2 ndcSnapped =
        (floor(((ndc + 1.0) * 0.5) * fbSize) + 0.5) * (2.0 / fbSize) - 1.0;

    // Back to clip space
    clipPos.xy = ndcSnapped * clipPos.w;

    gl_Position = clipPos;
}