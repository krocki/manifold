#version 330

uniform mat4 mvp;
in vec3 position;
in vec3 color;
in vec3 texcoords;
out vec4 fcolor;
out vec4 pos;
out vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

void main() {

	vec4 pos = mvp * vec4 ( position.x, position.y, 0.0, 1.0 );
	uv = texcoords.xy;

	fcolor = vec4(color, 1.0);
	gl_Position = pos;

}
