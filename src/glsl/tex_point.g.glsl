#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

in vec3 out_v_color[];
out vec3 out_g_color;

in vec2 uv[];
out vec2 out_g_tex;

uniform float radius;
uniform float tex_w;


void main (void) {

	vec4 P = gl_in[0].gl_Position;

	// a: left-bottom
	vec2 va = P.xy + vec2(-0.5, -0.5) * radius;
	gl_Position = proj * vec4(va, P.zw);
	out_g_tex = vec2(0.0, 0.0);
	out_g_color = out_v_color[0];
	EmitVertex();

	// b: left-top
	vec2 vb = P.xy + vec2(-0.5, 0.5) * radius;
	gl_Position = proj * vec4(vb, P.zw);
	out_g_tex = vec2(0.0, tex_w);
	out_g_color = out_v_color[0];
	EmitVertex();

	// d: right-bottom
	vec2 vd = P.xy + vec2(0.5, -0.5) * radius;
	gl_Position = proj * vec4(vd, P.zw);
	out_g_tex = vec2(tex_w, 0.0);
	out_g_color = out_v_color[0];
	EmitVertex();

	// c: right-top
	vec2 vc = P.xy + vec2(0.5, 0.5) * radius;
	gl_Position = proj * vec4(vc, P.zw);
	out_g_tex = vec2(tex_w, tex_w);
	out_g_color = out_v_color[0];
	EmitVertex();
}

// layout (points) in;
// layout (triangle_strip, max_vertices = 6) out;

// in vec3 out_v_color[];
// out vec3 out_g_color;

// in vec2 uv[];
// out vec2 out_g_tex;

// uniform float radius;
// uniform float tex_w;
// uniform mat4 proj;
// uniform mat4 view;

// void main() {


// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(-radius, -radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0] + vec2(0, tex_w);
// 	EmitVertex();

// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0];
// 	EmitVertex();

// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0] + vec2(tex_w, tex_w);
// 	EmitVertex();

// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(radius, radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0] + vec2(tex_w, 0);
// 	EmitVertex();

// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0];
// 	EmitVertex();

// 	gl_Position = proj * view * (gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0));
// 	out_g_color = out_v_color[0];
// 	out_g_tex = uv[0] + vec2(tex_w, tex_w);
// 	EmitVertex();

// 	EndPrimitive();

// }