#version 330

uniform mat4 mvp;
in vec3 color;
in vec3 position;
out vec3 out_v_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

void main() {

	gl_Position = mvp * vec4 ( position, 1.0 );
	out_v_color = color;
}
