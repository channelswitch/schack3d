#version 120

varying vec4 white;
varying vec3 transformedNormal;

varying float diffuseLight;
void main(void)
{
	gl_FragColor = diffuseLight*white;
}
