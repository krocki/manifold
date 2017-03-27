#version 330

out vec4 out_color;
in vec3 out_g_color;
uniform sampler2D image;
in vec2 out_g_tex;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {

	vec4 in_color = vec4(out_g_color, 0.8);
	//out_color =  vec4(1.0, 1.0, 1.0, 1.0);
	vec4 col = texture ( image, out_g_tex );
	out_color = min ( in_color, vec4 ( col.x, col.x, col.x, 0.8 ) );

}
