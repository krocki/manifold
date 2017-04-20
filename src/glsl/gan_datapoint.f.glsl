#version 330

in vec3 frag_color;
out vec4 out_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

uniform float alpha;
uniform int apply_color;
uniform int colormap;
uniform int discretize;

void main() {

	if ( apply_color == 0 ) {
		out_color = vec4 ( frag_color, alpha );

	} else {
		// float intensity = ( frag_color.x + frag_color.y + frag_color.z );
		out_color = apply_colormap ( frag_color, colormap, alpha, discretize );
	}

}
