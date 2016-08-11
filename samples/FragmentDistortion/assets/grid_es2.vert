#version 100

uniform mat4 ciModelViewProjection;
attribute vec4 ciPosition;

void main( void )
{
	highp vec4 pos = ciModelViewProjection * ciPosition;
	gl_Position = pos;
}