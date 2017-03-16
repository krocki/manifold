#version 330

uniform mat4 mvp;
in vec3 position;
in vec3 color;
out vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

void main() {

	vec4 pos = mvp * vec4 ( position, 1.0 );
	uv = pos.xy;
	
	gl_Position = pos;
	
}
