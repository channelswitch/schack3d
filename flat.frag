#version 120

varying vec2 outTexCoord;
uniform vec4 color;
varying vec3 transformedNormal;
uniform sampler2D texUnit;

const vec3 light = vec3(0.15, 0.15, 0.2);

void main(void)
{
	float diffuseLight;
	diffuseLight = max(0.5, 1.3 * dot(normalize(transformedNormal), normalize(light)));
	gl_FragColor = vec4(diffuseLight * color.xyz, color.a);
}
