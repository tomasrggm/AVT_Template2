#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;


uniform vec4 l_pos[6];

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataOut[6];

void main () {

	vec4 pos = m_viewModel * position;
	for(int i = 0; i < 6; ++i){
		DataOut[i].normal = normalize(m_normal * normal.xyz);
		DataOut[i].lightDir = vec3(l_pos[i] - pos);
		DataOut[i].eye = vec3(-pos);
	}


	gl_Position = m_pvm * position;	
}