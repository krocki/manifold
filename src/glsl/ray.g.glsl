#version 330

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

in vec3 out_v_color[];
out vec3 out_g_color;

void main() {

	gl_Position = gl_in[0].gl_Position;
	out_g_color = out_v_color[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	out_g_color = out_v_color[1];
	EmitVertex();

	EndPrimitive();

}