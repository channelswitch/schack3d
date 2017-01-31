#version 120

uniform vec4 color;
varying vec3 transformedNormal;
uniform sampler2D texUnit;
varying vec2 texCoord;

uniform vec3 light;

void main(void)
{
	float diffuseLight;
	diffuseLight = max(0.2, dot(normalize(transformedNormal), normalize(light)));
	gl_FragColor = diffuseLight*texture2D(texUnit, texCoord);
}
