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

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform int diffMapCount;

vec4 diff, auxSpec;

float l_spotCutOff = 0.720f;
vec4 l_spotDir = vec4(0.0,0.0,-1.0,1.0);

float density = 0.05;

uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform sampler2D texmap; //billboard
uniform sampler2D texmap1; //table-cloth
uniform sampler2D texmap2; //cloth
uniform sampler2D texmap3; //fireworks
uniform sampler2D texmap4; //lens flare
uniform sampler2D texmap5; //stone
uniform sampler2D texNormalMap; //normal
uniform samplerCube cubeMap; //Skybox
uniform bool shadowMode; //Shadows
uniform int texMode;
uniform int fog;
uniform mat4 m_View;

uniform vec4 luzDirectional;
uniform vec4 luzDifusa;
uniform vec4 luzHolofote;
uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	vec3 skyboxTexCoord;
} DataIn[6];

in vec4 pos;

void main() {
	vec4 spec = vec4(0.0);
	vec3 n;
	vec4 accumulatedValue = vec4(0.0);
	vec4 texel = vec4(0.0);
	vec4 texel1 = vec4(0.0);
	vec4 texel2 = vec4(0.0);
	vec4 texel3 = vec4(0.0);
	vec4 texel4 = vec4(0.0);

	if (shadowMode) { //constant color
		colorOut = vec4(0.5, 0.5, 0.5, 1.0);
	}

	else {
		for (int i = 0; i < 6; ++i) {
			if (i < 6) {

				if (normalMap)
					n = normalize(2.0 * texture(texUnitNormalMap, DataIn[i].tex_coord).rgb - 1.0);  //normal in tangent space
				else if (texMode == 7)
					n = normalize(2.0 * texture(texNormalMap, DataIn[i].tex_coord).rgb - 1.0);  //normal in tangent space
				else
					n = normalize(DataIn[i].normal);

				//If bump mapping, normalMap == TRUE, then lightDir and eye vectores come from vertex shader in tangent space
				vec3 l = normalize(DataIn[i].lightDir);
				vec3 e = normalize(DataIn[i].eye);

				float intensity = max(dot(n, l), 0.0);

				if (mat.texCount == 0) {
					diff = mat.diffuse;
					auxSpec = mat.specular;
				}
				else {
					if (diffMapCount == 0)
						diff = mat.diffuse;
					else if (diffMapCount == 1)
						diff = mat.diffuse * texture(texUnitDiff, DataIn[i].tex_coord);
					else
						diff = mat.diffuse * texture(texUnitDiff, DataIn[i].tex_coord) * texture(texUnitDiff1, DataIn[i].tex_coord);

					if (specularMap)
						auxSpec = mat.specular * texture(texUnitSpec, DataIn[i].tex_coord);
					else
						auxSpec = mat.specular;
				}

				if (intensity > 0.0) {

					vec3 h = normalize(l + e);
					float intSpec = max(dot(h, n), 0.0);
					spec = auxSpec * pow(intSpec, mat.shininess);
					if (texMode == 1) {
						texel1 = texture(texmap1, DataIn[i].tex_coord);
						texel2 = texture(texmap2, DataIn[i].tex_coord);
						accumulatedValue += (spec + intensity * diff * texel1 * texel2) * luzDifusa;
					}
					else if (texMode == 2) {
						texel = texture(texmap, DataIn[i].tex_coord);
						if (texel.a == 0.0) discard;
						else {
							accumulatedValue += (spec + intensity * diff * texel) * luzDifusa;
							accumulatedValue[3] += texel.a;
						}
					}
					else if (texMode == 3) {
						texel3 = texture(texmap3, DataIn[i].tex_coord);
						if (texel3.a == 0.0 || (mat.diffuse.a == 0.0)) discard;
						else {
							accumulatedValue += (spec + intensity * diff * texel3) * luzDifusa;
							accumulatedValue[3] += texel3.a;
						}
					}
					else if (texMode == 6) {
						vec3 reflected1 = vec3(transpose(m_View) * vec4(vec3(reflect(-e, n)), 0.0));
						reflected1.x = -reflected1.x;
						texel = texture(cubeMap, reflected1);
						texel1 = texture(texmap1, DataIn[i].tex_coord);  // texel from lighwood.tga
						vec4 aux_color = mix(texel, texel, 1.0);
						aux_color = max(intensity * aux_color + spec, 0.1 * aux_color);
						accumulatedValue += vec4(aux_color.rgb, 1.0);
					}
					else if (texMode == 7) {
						texel4 = texture(texmap5, DataIn[i].tex_coord);
						accumulatedValue += (spec + intensity * diff * texel4) * luzDifusa;
					}
					else {
						accumulatedValue += (spec + intensity * diff) * luzDifusa;
					}

				}
			}
			else {
				float intensity = 0.0;
				vec4 spec = vec4(0.0);
				vec3 ld = normalize(DataIn[i].lightDir);
				vec3 sd = normalize(vec3(-l_spotDir));
				if (dot(sd, ld) > l_spotCutOff) {
					vec3 n = normalize(DataIn[i].normal);
					intensity = max(dot(n, ld), 0.0);
					if (intensity > 0.0) {
						vec3 eye = normalize(DataIn[i].eye);
						vec3 h = normalize(ld + eye);
						float intSpec = max(dot(h, n), 0.0);
						spec = auxSpec * pow(intSpec, mat.shininess);
						if (texMode == 1) {
							texel1 = texture(texmap1, DataIn[i].tex_coord);
							texel2 = texture(texmap2, DataIn[i].tex_coord);
							accumulatedValue += (spec + intensity * diff * texel1 * texel2) * luzHolofote;
						}
						else if (texMode == 2) {
							texel = texture(texmap, DataIn[i].tex_coord);
							if (texel.a == 0.0) discard;
							else {
								accumulatedValue += (spec + intensity * diff * texel) * luzHolofote;
								accumulatedValue[3] += texel.a;
							}
						}
						else if (texMode == 3) {
							texel3 = texture(texmap3, DataIn[i].tex_coord);
							if (texel3.a == 0.0 || (mat.diffuse.a == 0.0)) discard;
							else {
								accumulatedValue += (spec + intensity * diff * texel3) * luzHolofote;
								accumulatedValue[3] += texel3.a;
							}
						}
						else if (texMode == 7) {
							texel4 = texture(texmap5, DataIn[i].tex_coord);
							accumulatedValue += (spec + intensity * diff * texel4) * luzHolofote;
						}
						else {
							accumulatedValue += (spec + intensity * diff) * luzHolofote;
						}

					}
				}
				else {
					vec3 n = normalize(DataIn[i].normal);
					intensity = max(dot(n, ld), 0.0);
					if (intensity > 0.0) {
						vec3 eye = normalize(DataIn[i].eye);
						vec3 h = normalize(ld + eye);
						float intSpec = max(dot(h, n), 0.0);
						spec = auxSpec * pow(intSpec, mat.shininess);
						if (texMode == 2) {
							texel = texture(texmap, DataIn[i].tex_coord);
							if (texel.a == 0.0) discard;
							else {
								accumulatedValue += (spec + intensity * diff * texel) * luzHolofote;
								accumulatedValue[3] += texel.a;
							}
						}
						else if (texMode == 3) {
							texel3 = texture(texmap3, DataIn[i].tex_coord);
							if (texel3.a == 0.0 || (mat.diffuse.a == 0.0)) discard;
							else {
								accumulatedValue += (spec + intensity * diff * texel3) * luzHolofote;
								accumulatedValue[3] += texel3.a;
							}
						}
					}
				}
			}

		}

		if (texMode == 1) { //Ensure directional light applies to the table textures
			colorOut = max(accumulatedValue, mat.ambient * luzDirectional * texel1 * texel2);
			colorOut[3] = mat.diffuse.a;
		}

		else if (texMode == 2) { //Ensure directional light applies to the tree billboards
			if (texel.a == 0.0) discard;
			else {
				colorOut = max(accumulatedValue, mat.ambient * luzDirectional * texel);
				colorOut[3] = texel.a;
			}
		}

		else if (texMode == 3) { //Ensure directional light applies to the fireworks
			if(texel3.a == 0 || (mat.diffuse.a == 0.0)) discard;
			else {
				colorOut = max(accumulatedValue, mat.ambient * luzDirectional * texel3);
				colorOut[3] = texel3.a;
			}
		}

		else if (texMode == 4) {
			vec4 texel3 = vec4(0.0);
			for (int i = 0; i < 6; ++i) {
				texel3 += texture(cubeMap, DataIn[i].skyboxTexCoord);
			}
			colorOut = texel3;
		}

		else if (texMode == 5) {
			texel2 = texture(texmap4, DataIn[5].tex_coord);
			if ((texel2.a == 0.0) || (mat.diffuse.a == 0.0)) discard;
			else
				colorOut = texel2 * mat.diffuse;
		}

		else if (texMode == 6) {
			colorOut = accumulatedValue;
		}

		else if (texMode == 7) {
			colorOut = max(accumulatedValue, mat.ambient * luzDirectional * texel4);
		}

		else {
			colorOut = max(accumulatedValue, mat.ambient * luzDirectional);
		}
	}

	vec3 fogColor = vec3(0.5, 0.6, 0.7); //define fog color

	float distance = length(pos); //range based
	//float distance = abs(pos.z); //plane based

	float visibility = exp(-distance * density); //fog function
	//visibility = clamp(visibility, 0.0, 1.0);

	//apply fog
	colorOut[3] = mat.diffuse.a;
	if (fog == 1) {
		colorOut = mix(vec4(fogColor, 1.0), colorOut, visibility);
	}

}