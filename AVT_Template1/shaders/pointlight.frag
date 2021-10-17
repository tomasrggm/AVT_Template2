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

float l_spotCutOff = 0.720f;
vec4 l_spotDir = vec4(0.0,0.0,-1.0,1.0);
int depthFog = 1;
float dist = 0.0f;

uniform vec4 luzAmbiente;
uniform vec4 luzDifusa;
uniform vec4 luzHolofote;
uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataIn[8];

in vec4 position;
in float visibility;

void main() {
	vec4 spec = vec4(0.0);
	vec4 accumulatedValue = vec4(0.0);

	for(int i = 0; i < 8; ++i) {
		if(i < 6) {
			vec3 n = normalize(DataIn[i].normal);
			vec3 l = normalize(DataIn[i].lightDir);
			vec3 e = normalize(DataIn[i].eye);

			float intensity = max(dot(n,l), 0.0);

	
			if (intensity > 0.0) {

				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
				accumulatedValue +=(spec + intensity*mat.diffuse)* luzDifusa;
			}
		}
		else {
			float intensity = 0.0;
			vec4 spec = vec4(0.0);
			vec3 ld = normalize(DataIn[i].lightDir);
			vec3 sd = normalize(vec3(-l_spotDir));
			if (dot(sd,ld) > l_spotCutOff) {
				vec3 n = normalize(DataIn[i].normal);
				intensity = max(dot(n,ld), 0.0);
				if (intensity > 0.0) {
					vec3 eye = normalize(DataIn[i].eye);
					vec3 h = normalize(ld + eye);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess);
					accumulatedValue += (spec + intensity*mat.diffuse) * luzHolofote;
				}
			}
		}

	}

	vec3 fogColor = vec3(0.5,0.6,0.7); //define fog color

	colorOut = max(accumulatedValue , mat.ambient * luzAmbiente);
	colorOut = mix(vec4(fogColor,1.0), colorOut, visibility); //apply fog
}