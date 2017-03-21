#version 330

uniform mat4 mvp;
in vec3 position;
in vec3 color;
out vec4 fcolor;
/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {
	gl_Position = mvp * vec4 ( position, 1.0 );
	fcolor = vec4(color, 1.0);
}