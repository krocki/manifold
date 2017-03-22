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
	float distance = sqrt(dot ( circ, circ ));

	// if ( distance > 1.0 )
	// 	discard;

	// if ( frag_color == vec3 ( 0.0 ) )
	// 	discard;

	float circle_radius = 0.1f;
	float border = 1.0f;

	float t = 1.0 + smoothstep(circle_radius, circle_radius + border, distance) - smoothstep(circle_radius - border, circle_radius, distance);

	out_color = vec4 ( mix(vec4(frag_color, 0.05f), vec4(0.0f), t ));

}
