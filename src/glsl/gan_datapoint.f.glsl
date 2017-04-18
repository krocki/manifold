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
uniform int blend_mode;
uniform int colormap;

void main() {

	float intensity = ( frag_color.x + frag_color.y + frag_color.z );
	if ( blend_mode == 1 ) { // GL_ONE, GL_ONE
		// out_color = vec4 ( frag_color * alpha, intensity );
		out_color = vec4 ( frag_color * alpha, intensity ); //hueGradient(float t)
	} else {

		out_color = apply_colormap(frag_color, colormap, alpha);

	}

}
