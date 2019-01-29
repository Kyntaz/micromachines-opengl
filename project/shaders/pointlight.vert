#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;
uniform mat4 model;
uniform mat4 view;

uniform vec4 l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec4 position;
	vec2 tex_coord;
	vec4 pos;
} DataOut;


void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(inverse(transpose(mat3(model))) * normal.xyz);
	DataOut.eye = (inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	DataOut.position = model * position;
	DataOut.tex_coord = texCoord.st;
	DataOut.pos = pos;
	gl_Position = m_pvm * position;	
}