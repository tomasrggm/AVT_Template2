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

float density = 0.05;

uniform sampler2D texmap; //tree
uniform sampler2D texmap1; //table-cloth
uniform sampler2D texmap2; //cloth
uniform int texMode;

uniform vec4 luzDirectional;
uniform vec4 luzDifusa;
uniform vec4 luzHolofote;
uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataIn[8];

in vec4 pos;

void main() {
	vec4 spec = vec4(0.0);
	vec4 accumulatedValue = vec4(0.0);
	vec4 texel = vec4(0.0);
	vec4 texel1 = vec4(0.0);
	vec4 texel2 = vec4(0.0);

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
				if(texMode == 0) {
					texel1 = texture(texmap1, DataIn[i].tex_coord);
					texel2 = texture(texmap2, DataIn[i].tex_coord);
					accumulatedValue +=(spec + intensity*mat.diffuse*texel1*texel2)* luzDifusa;
				}
				else if(texMode == 1) {
					texel = texture(texmap, DataIn[i].tex_coord);
					if(texel.a == 0.0) discard;
					else {
						accumulatedValue += (spec + intensity*mat.diffuse*texel) * luzDifusa;
						accumulatedValue[3] += texel.a;
					}
				}
				else {
					accumulatedValue +=(spec + intensity*mat.diffuse)* luzDifusa;
				}
				
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
					if(texMode == 0) {
						texel1 = texture(texmap1, DataIn[i].tex_coord);
						texel2 = texture(texmap2, DataIn[i].tex_coord);
						accumulatedValue += (spec + intensity*mat.diffuse*texel1*texel2) * luzHolofote;
					}
					else if(texMode == 1) {
						texel = texture(texmap, DataIn[i].tex_coord);
						if(texel.a == 0.0) discard;
						else {
							accumulatedValue += (spec + intensity*mat.diffuse*texel) * luzHolofote;
							accumulatedValue[3] += texel.a;
						}
					}
					else {
						accumulatedValue += (spec + intensity*mat.diffuse) * luzHolofote;
					}
					
				}
			}
		}

	}

	vec3 fogColor = vec3(0.5,0.6,0.7); //define fog color

	float distance = length(pos); //range based
	//float distance = abs(pos.z); //plane based

	float visibility = exp(-distance*density); //fog function
	//visibility = clamp(visibility, 0.0, 1.0);

	if(texMode == 0) { //Ensure directional light applies to the table textures
		colorOut = max(accumulatedValue , mat.ambient * luzDirectional * texel1 * texel2);
		colorOut[3] = mat.diffuse.a;
	}

	else if(texMode == 1) { //Ensure directional light applies to the tree billboards
		if(texel.a == 0.0) discard;
		colorOut = max(accumulatedValue , mat.ambient * luzDirectional * texel);
		colorOut[3] = texel.a;
	}

	else {
		colorOut = max(accumulatedValue , mat.ambient * luzDirectional);
		colorOut[3] = mat.diffuse.a; 
	}

	colorOut = mix(vec4(fogColor,1.0), colorOut, visibility); //apply fog
}