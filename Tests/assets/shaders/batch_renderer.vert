#version 330 core

//Vertex
layout(location = 0) in vec2 iVertexPos;  // base quad corner [-0.5, 0.5]
//Instance
layout(location = 1) in float iType;
layout(location = 2) in vec3 iPos;      
layout(location = 3) in vec4 iUvOrColor;      
layout(location = 4) in vec2 iSize;        
layout(location = 5) in float iRotation;      
layout(location = 6) in float iTexIndex;       
layout(location = 7) in float iAlpha;

out vec2 vTexCoords;
out vec4 vColor;
out float vTexIndex;

layout(std140) uniform camera {
    mat4 uViewProjection;
    float uWorldScreenRatio;
};

void main() {
    // Scale quad to sprite size & transform scale
    vec3 scaled = vec3(iVertexPos.x * iSize.x,
                       iVertexPos.y * iSize.y,
                       0);

    float rot = radians(iRotation);

    // Apply rotation
    float s = sin(rot);
    float c = cos(rot);
    vec3 rotated = vec3(c * scaled.x - s * scaled.y,
                        s * scaled.x + c * scaled.y, 
                        0);

    // Position in world space
    vec3 worldPos = rotated + iPos;

    // Apply world-to-screen ratio and pixel per unit
    vec3 finalPos = worldPos * uWorldScreenRatio;

    // Compute local UV (map [-0.5, 0.5] → [0, 1])
    vec2 localUV = iVertexPos.xy + 0.5;

    if(iType == 0)
    {
        //Untextured colored Quad
        vColor = iUvOrColor;
        vTexCoords = localUV;
        vTexIndex = 0;
    }
    else
    {
        //Sprite
        //Interpolate between min and max UV
        vec2 uvMin = vec2(iUvOrColor.x, iUvOrColor.y);
        vec2 uvMax = vec2(iUvOrColor.z, iUvOrColor.w);
        vTexCoords = mix(uvMin, uvMax, localUV);
        vColor = vec4(1, 1, 1, iAlpha);
        vTexIndex = iTexIndex;
    }

    gl_Position = uViewProjection * vec4(finalPos, 1.0);
}