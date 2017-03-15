#version 330

uniform mat4 mvp;
in vec3 position;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

smooth out vec4 fragment_in_color;

void main() {
	gl_Position = mvp * vec4 ( position, 1.0 );
	float d = distance ( position, vec3 ( 0, 0, 0 ) );
	fragment_in_color = vec4 ( 1.0f, 1.0f, 1.0f, 0.5 / ( 1.0f + 5 * d ) );
	// fragment_in_color.a = 0.25;
}