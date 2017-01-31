#version 120

attribute  vec3 in_Position;
attribute  vec3 in_Normal;
uniform vec4 color;
varying vec4 red;

uniform mat4 rotMatrix;
uniform  mat4 projectionMatrix;
uniform  mat4 mdlMatrix;

varying vec3 transformedNormal;


void main(void)
{
	mat3 normalMatrix = mat3(rotMatrix);
	transformedNormal = normalMatrix * in_Normal;
	gl_Position = projectionMatrix*mdlMatrix*rotMatrix*vec4(in_Position, 1.0);
	
}
