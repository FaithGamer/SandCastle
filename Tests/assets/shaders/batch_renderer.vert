#version 330 core

//Vertex
layout(location = 0) in vec3 iVertexPos;  // base quad corner [-0.5, 0.5]
layout(location = 1) in vec2 iUv;      
layout(location = 2) in vec4 iColor;      
layout(location = 3) in float iTexIndex;       


out vec2 vTexCoords;
out vec4 vColor;
out float vTexIndex;

layout(std140) uniform camera {
    mat4 uViewProjection;
    float uWorldScreenRatio;
};

void main() 
{
    vColor = iColor;
    vTexCoords = iUv;
    vTexIndex = iTexIndex;
  
    gl_Position = uViewProjection * vec4(iVertexPos, 1.0);
}