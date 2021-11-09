#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;
uniform mat4 m_Model;

uniform vec4 l_pos[8];
uniform bool normalMap;

in vec4 normal, tangent, bitangent;    //por causa do gerador de geometria
in vec4 position;
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	vec3 skyboxTexCoord;
} DataOut[6];

out vec4 pos;

void main () {
	
	vec3 n, t, b;
	vec3 lightDir, eyeDir;
	vec3 aux;

	pos = m_viewModel * position;
	for(int i = 0; i < 6; ++i) {
		n = normalize(m_normal * normal.xyz);
		lightDir = vec3(l_pos[i] - pos);
		eyeDir = vec3(-pos);
		
		if(normalMap)  {  //transform eye and light vectors by tangent basis
			t = normalize(m_normal * tangent.xyz);
			b = normalize(m_normal * bitangent.xyz);

			aux.x = dot(lightDir, t);
			aux.y = dot(lightDir, b);
			aux.z = dot(lightDir, n);
			lightDir = normalize(aux);

			aux.x = dot(eyeDir, t);
			aux.y = dot(eyeDir, b);
			aux.z = dot(eyeDir, n);
			eyeDir = normalize(aux);
		}

		DataOut[i].normal = n;
		DataOut[i].lightDir = lightDir;
		DataOut[i].eye = eyeDir;
		DataOut[i].tex_coord = texCoord.st;
		DataOut[i].skyboxTexCoord = vec3(m_Model * position);	//Transformação de modelação do cubo unitário 
		DataOut[i].skyboxTexCoord.x = - DataOut[i].skyboxTexCoord.x; //Texturas mapeadas no interior logo negar a coordenada x
	}


	gl_Position = m_pvm * position;	
}