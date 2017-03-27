#version 330

in vec3 position;
in vec3 color;
out vec3 out_v_color;

in vec3 texcoords;
out vec4 pos;
out vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/


uniform mat4 model;
uniform mat4 view;

void main() {

	vec4 pos = vec4 ( position.x, position.y, position.z, 1.0 );

	uv = texcoords.xy;

	out_v_color = vec3(color);
	gl_Position = view * model * pos;

}