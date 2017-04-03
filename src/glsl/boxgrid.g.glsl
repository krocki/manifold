#version 330

layout (lines) in;
layout (line_strip, max_vertices = 250) out;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

in vec4 gcolor[];
out vec4 fcolor;

void main() {

	vec4 pos_a = (gl_in[0].gl_Position);
	vec4 pos_b = (gl_in[1].gl_Position);

	float spacing = 0.0f;

	pos_a.xyz -= (10);
	pos_b.xyz -= (10);

	pos_a.xyz /= (5.0f + spacing);
	pos_b.xyz /= (5.0f + spacing);

	for (int i = 0; i < 5; i++) {

		for (int j = 0; j < 5; j++) {

			for (int k = 0; k < 5; k++) {

				gl_Position = (proj * view * model) * (pos_a + vec4(i, j, k, 0));
				fcolor = gcolor[0];
				fcolor.a = gcolor[0].a / 5;
				EmitVertex();

				gl_Position = (proj * view * model) * (pos_b + vec4(i, j, k, 0));
				fcolor = gcolor[1];
				fcolor.a = gcolor[1].a / 5;
				EmitVertex();

				EndPrimitive();
			}

		}

	}



}
