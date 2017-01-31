#version 120

attribute  vec3 in_Position;
attribute  vec3 in_Normal;
uniform vec4 color;
attribute  vec2 in_TexCoord;

uniform mat4 rotMatrix;
uniform  mat4 projectionMatrix;
uniform  mat4 mdlMatrix;

varying vec3 transformedNormal;
varying vec2 texCoord;


void main(void)
{
	mat3 normalMatrix = mat3(rotMatrix);
	transformedNormal = normalMatrix * in_Normal;
	gl_Position = projectionMatrix*mdlMatrix*rotMatrix*vec4(in_Position, 1.0);
	texCoord = in_TexCoord;
}
