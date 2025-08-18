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
    vColor = iColor;
    vTexCoords = iUv;
    vTexIndex = iTexIndex;
    
    vec3 pos = iVertexPos.xyz * uCamZoom;
    gl_Position = uCamProj  * vec4(pos, 1.0);
}