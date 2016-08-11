#version 150

#include "VertexDistortion"

uniform mat4 ciModelViewProjection;
in vec4 ciPosition;

void main( void )
{
	vec4 pos = ciModelViewProjection * ciPosition;
	gl_Position = Distort(pos);
}