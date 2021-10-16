#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos[8];

in vec4 normal;    //por causa do gerador de geometria

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataOut[8];

in vec4 position;
out float visibility;

const float density = 0.055;

void main () {

	vec4 pos = m_viewModel * position;
	for(int i = 0; i < 8; ++i){
		DataOut[i].normal = normalize(m_normal * normal.xyz);
		DataOut[i].lightDir = vec3(l_pos[i] - pos);
		DataOut[i].eye = vec3(-pos);
	}

	float distance = length(pos); //range based
	//float distance = abs(pos.z); //plane based
	visibility = exp(-distance*density);
	//visibility = clamp(visibility, 0.0, 1.0);


	gl_Position = m_pvm * position;	
}