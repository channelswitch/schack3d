#version 120

attribute  vec3 in_Position;
attribute  vec3 in_Normal;
uniform  vec4 color;
varying vec4 white;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 translMatrix;

vec3 transformedNormal;
varying float diffuseLight;

const vec3 light = vec3(-0.18, 0.58, 0.58);

void main(void)
{
	mat3 normalMatrix = mat3(mdlMatrix);
	transformedNormal = normalMatrix * in_Normal;

	diffuseLight = max(0.2, dot(normalize(transformedNormal), normalize(light)));

	gl_Position = projMatrix*mdlMatrix*translMatrix*vec4(in_Position, 1.0);
	white = color;
}
