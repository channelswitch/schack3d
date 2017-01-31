#version 120

varying vec4 white;
//in float diffuseLight;
varying vec3 transformedNormal;
//out float diffuseLight;

const vec3 light = vec3(0.98, 0.98, 0.98);

void main(void)
{
	
	//Light
//	diffuseLight = max(0.0, dot(normalize(transformedNormal), light));
	//gl_FragColor = vec4(0.5, .8, 1.0, 0.0);
	//gl_FragColor = color_bla;
	//float a = cos(outTexCoord.s*30)/2+0.5;
	//float b = cos(outTexCoord.t*30)/2+0.5;
	//gl_FragColor = diffuseLight*colors;
	gl_FragColor = white;
}
