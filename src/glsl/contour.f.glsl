#version 330

in vec4 frag_color;
out vec4 out_color;
uniform sampler2D image;
in vec2 tex;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {

	vec2 uv = gl_PointCoord;
	vec2 circ = 2.0 * uv.xy - 1.0;
	float distance = dot ( circ, circ );

	if ( distance > 1.0 )
		discard;

	vec4 col = texture ( image, tex );
	// // if ( frag_color == vec3 ( 0.0 ) )
	// // 	discard;

	// float circle_radius = 0.0f;
	// float border = 0.3f;
	// float t = 1.0 + smoothstep(circle_radius, circle_radius + border, distance) - smoothstep(circle_radius - border, circle_radius, distance);

	// out_color = vec4 ( mix(vec4(frag_color), vec4(0.0f), t ));

	out_color = frag_color;
}
