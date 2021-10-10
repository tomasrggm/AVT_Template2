#version 330

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};


uniform vec4 luzAmbiente;
uniform vec4 luzDifusa;
uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataIn[6];

const int MaxLights = 10;


void main() {

	vec4 spec = vec4(0.0);
	vec4 accumulatedValue = vec4(0.0);

	for(int i = 0; i < 6; ++i){
		vec3 n = normalize(DataIn[i].normal);
		vec3 l = normalize(DataIn[i].lightDir);
		vec3 e = normalize(DataIn[i].eye);

		float intensity = max(dot(n,l), 0.0);

	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
			accumulatedValue +=(spec + intensity*mat.diffuse);
		}
	}

	
	colorOut = max(accumulatedValue * luzDifusa, mat.ambient * luzAmbiente);
}