#version 330 core
#ifndef PI
#define PI 3.141592653589793
#endif

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices=50) out;


layout (std140) uniform camera
{
	mat4 uViewProjection;
	float uWorldToScreen;
};

in float vIndex[];
const int nbWidth = 5;
uniform float uWidth[nbWidth];
uniform float uAspectRatio;
uniform float uIndexCount;
uniform int uEndCapVertices;


float sineInOut(float t) 
{
	return -0.5 * (cos(PI * t) - 1.0);
}

vec2 offsetDir(vec2 dirIn, vec2 dirOut, float width)
{
	float w = clamp((1 - dot(dirIn, dirOut)) * 1.41, 1, 1.41);
	vec2 c = vec2(cross(vec3(-dirIn.xy, 1), vec3(dirOut.xy, 1)));
	c.x /= uAspectRatio;
	return normalize(c) * width * uWorldToScreen * 0.5 * w;
}

float width(float index, float count)
{
	float nb = float(nbWidth);
	float increment = 1.0 / (nb-1);

	float progression = index / (count-1);

	float next = ceil(progression / increment);
	float prev = max(0, next-1);

	float curProg = (progression - (prev*increment)) / increment;

	return mix(uWidth[int(prev)], uWidth[int(next)], sineInOut(curProg));

	//return mix(prev, next, curProg);
}

void drawCircle(int index, vec2 dir90, float width)
{
	float angleIncr = PI/float(uEndCapVertices);
	for(int i = 0; i < uEndCapVertices; i++)
	{
		float angle = angleIncr * float(i) + atan(dir90.y, dir90.x);
		vec2 p = normalize(vec2(cos(angle), sin(angle))) * width * uWorldToScreen * 0.5;

		p.x /= uAspectRatio;
		gl_Position = gl_in[index].gl_Position;
		EmitVertex();
		gl_Position = gl_in[index].gl_Position + vec4(p.xy, 0, 0);
		EmitVertex();
	}
}

void main()
{

	vec2 dirIn = gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy;
	vec2 dir =  gl_in[2].gl_Position.xy - gl_in[1].gl_Position.xy;
	vec2 dirOut =  gl_in[3].gl_Position.xy - gl_in[2].gl_Position.xy;

	float widthBegin = width(vIndex[1], uIndexCount);
	float widthEnd = width(vIndex[2], uIndexCount);

	//begin of line
	if(length(dirIn) < 0.0001)
	{
		dirIn = dir;
		vec2 offset = offsetDir(normalize(dirIn), normalize(dir), widthBegin);
		offset.x /= uAspectRatio;
		
		vec2 dir90 = normalize(vec2(-dir.y, dir.x));

		gl_Position = gl_in[1].gl_Position + vec4(offset.xy, 0, 0);
		EmitVertex();
		drawCircle(1, dir90, widthBegin);
		gl_Position = gl_in[1].gl_Position - vec4(offset.xy, 0, 0);
		EmitVertex();
	}

	//end of line
	bool drawEnd = false;
	if(length(dirOut) < 0.0001)
	{
		dirOut =  dir;
		drawEnd = true;
	}
	
	//for cross product to work
	dirIn = normalize(dirIn);
	dirOut = normalize(dirOut);
	dir = normalize(dir);

	vec2 startOffset = offsetDir(dirIn, dir, widthBegin);

	vec2 endOffset = offsetDir(dir, dirOut, widthEnd);
	startOffset.x /= uAspectRatio;
	endOffset.x /= uAspectRatio;

	gl_Position = gl_in[1].gl_Position + vec4(startOffset.xy, 0, 0);
	EmitVertex();
	gl_Position = gl_in[1].gl_Position - vec4(startOffset.xy, 0, 0);
	EmitVertex();
	gl_Position = gl_in[2].gl_Position + vec4(endOffset.xy, 0, 0);
	EmitVertex();
	gl_Position = gl_in[2].gl_Position - vec4(endOffset.xy, 0, 0);
	EmitVertex();

	if(drawEnd)
	{
		vec2 dir90 = normalize(vec2(-dir.y, dir.x));

		drawCircle(2, -dir90, widthEnd);
		gl_Position = gl_in[2].gl_Position + vec4(endOffset.xy, 0, 0);
		EmitVertex();
	}


	EndPrimitive();
}