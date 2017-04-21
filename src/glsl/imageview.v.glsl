#version 330

uniform vec2 scaleFactor;
uniform vec2 position;
in vec2 vertex;
out vec2 uv;

void main() {
	uv = vertex;
	vec2 scaledVertex = ( vertex * scaleFactor ) + position;
	gl_Position  = vec4 ( 2.0 * scaledVertex.x - 1.0,
						  1.0 - 2.0 * scaledVertex.y,
						  0.0, 1.0 );
						  
}