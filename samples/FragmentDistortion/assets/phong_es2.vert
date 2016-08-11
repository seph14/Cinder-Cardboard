#version 100

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;
uniform mat3 ciNormalMatrix;

attribute vec4 ciPosition;
attribute vec3 ciNormal;

varying highp vec4 vWorldPosition;
varying highp vec3 vNormal;

void main( void )
{
	vWorldPosition = ciModelMatrix * ciPosition;
	vNormal = ciNormalMatrix * ciNormal;
	highp vec4 pos = ciModelViewProjection * ciPosition;
	gl_Position = pos;
}