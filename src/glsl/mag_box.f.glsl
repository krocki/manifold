#version 330

out vec4 out_color;
uniform sampler2D image;
in vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {
	out_color = texture ( image, uv );
}
