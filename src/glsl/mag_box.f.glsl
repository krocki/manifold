#version 330

in vec3 frag_color;
out vec4 out_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {
	if ( frag_color == vec3 ( 0.0 ) )
		discard;
	out_color = vec4 ( frag_color, 1.0 );
}
