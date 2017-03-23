#version 330

in vec3 out_g_color;
out vec4 out_color;
uniform sampler2D image;
in vec2 out_g_tex;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {

	vec4 col = texture ( image, out_g_tex );
	// out_color = vec4(out_g_color, 0.5f);
	out_color = col;

}
