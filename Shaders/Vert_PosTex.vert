#version 430

layout( location = 0 ) in vec3 vs_in_pos;
layout( location = 1 ) in vec2 vs_in_tex;

out vec3 vs_out_pos;
out vec2 vs_out_tex;

void main()
{
	gl_Position = vec4(vs_in_pos, 1.0);
	vs_out_pos = vs_in_pos;
	vs_out_tex = vs_in_tex;
}