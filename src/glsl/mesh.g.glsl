#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 out_v_color[];
out vec3 out_g_color;

in vec2 uv[];
out vec2 out_g_tex;

void main() {

	gl_Position = gl_in[0].gl_Position;
	out_g_color = out_v_color[0];
	out_g_tex = uv[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	out_g_color = out_v_color[1];
	out_g_tex = uv[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	out_g_color = out_v_color[2];
	out_g_tex = uv[2];
	EmitVertex();

	EndPrimitive();

}