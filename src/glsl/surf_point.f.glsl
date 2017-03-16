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

	vec2 p = gl_PointCoord;
	vec2 circ = 2.0 * p - 1.0;
	float distance = dot ( circ, circ );
	
	if ( distance > 1.0 )
		discard;
		
	if ( frag_color == vec3 ( 0.0 ) )
		discard;
		
	out_color = vec4 ( frag_color, 1.0 - sqrt ( distance ) );
}
