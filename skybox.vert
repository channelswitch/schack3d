#version 120

attribute  vec3 in_Position;
attribute  vec3 inNormal;
attribute vec2 inTexCoord;
varying vec2 texCoord;

// NY
uniform mat4 projMatrix;
uniform mat4 mdlMatrix;

void main(void)
{
	mat3 normalMatrix1 = mat3(mdlMatrix);
	texCoord = inTexCoord;
	gl_Position = projMatrix * mdlMatrix * vec4(in_Position, 1.0);
}
