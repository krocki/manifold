#version 330

in vec4 frag_color;
out vec4 out_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {

	// vec2 uv = gl_PointCoord;
	// vec2 circ = 2.0 * uv - 1.0;
	// float distance = sqrt(dot ( circ, circ ));
	
	// if ( distance > 1.0 )
	// 	discard;
	
	out_color = vec4 ( frag_color );
}
