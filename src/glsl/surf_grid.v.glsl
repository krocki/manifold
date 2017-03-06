#version 410

uniform mat4 mvp;
in vec3 position;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {
	gl_Position = mvp * vec4(position, 1.0);
}