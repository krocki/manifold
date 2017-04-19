#version 330

layout ( lines ) in;
layout ( line_strip, max_vertices = 100 ) out;

uniform mat4 mvp;

in vec4 gcolor[];
out vec4 fcolor;

void main() {

	gl_Position = mvp * ( gl_in[0].gl_Position );
	fcolor = gcolor[0];
	EmitVertex();
	
	gl_Position = mvp * ( gl_in[1].gl_Position );
	fcolor = gcolor[1];
	EmitVertex();
	
	EndPrimitive();
	
	// vec4 a = gl_in[0].gl_Position;
	// vec4 b = gl_in[1].gl_Position;
	
	// for (int i = 0; i < 20; i++) {
	// 	a.y += 2;
	// 	b.y += 2;
	// 	if (a.y < 11 && b.y < 11) {
	// 		gl_Position = mvp * (a);
	// 		fcolor.a = gcolor[0].a / 4;
	// 		EmitVertex();
	
	// 		gl_Position = mvp * ( b );
	// 		fcolor.a = gcolor[1].a / 4;
	// 		EmitVertex();
	// 		EndPrimitive();
	// 	}
	// }
	
	// a = gl_in[0].gl_Position;
	// b = gl_in[1].gl_Position;
	
	// for (int i = 0; i < 20; i++) {
	// 	a.x += 2;
	// 	b.x += 2;
	// 	if (a.x < 11 && b.x < 11) {
	// 		gl_Position = mvp * (a);
	// 		fcolor.a = gcolor[0].a / 8;
	// 		EmitVertex();
	
	// 		gl_Position = mvp * ( b );
	// 		fcolor.a = gcolor[1].a / 8;
	// 		EmitVertex();
	// 		EndPrimitive();
	// 	}
	// }
	
	
}
