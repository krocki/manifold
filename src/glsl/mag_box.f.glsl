#version 330

out vec4 out_color;
in vec4 fcolor;
uniform sampler2D image;
in vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {

	vec4 col = texture ( image, uv );
	out_color = min ( fcolor, vec4 ( col.x, col.x, col.x, 0.9 ) );
	
}
