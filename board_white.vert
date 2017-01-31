#version 120

attribute  vec3 in_Position;
attribute  vec3 in_Normal;
uniform  vec4 color;
varying vec4 white;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 translMatrix;

//uniform  mat4 projectionMatrix;
//uniform  mat4 lookAtMatrix;
varying vec3 transformedNormal;


void main(void)
{
	mat3 normalMatrix = mat3(mdlMatrix);
	transformedNormal = normalMatrix * in_Normal;

 
	
	//colors = vec4(in_Normal+0.4, 1.0);
	//colors = inNormal;
	gl_Position = projMatrix*mdlMatrix*translMatrix*vec4(in_Position, 1.0);
	//gl_Position = projectionMatrix*translationMatrix*rotationMatrix*myMatrix1*myMatrix2*vec4(in_Position, 1.0);
	//gl_Position = vec4(inPosition, 1.0);
	white = color;
}
