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

float colormap_red ( float x ) {
	if ( x < 0.7 )
		return 4.0 * x - 1.5;
	else
		return -4.0 * x + 4.5;
}

float colormap_green ( float x ) {
	if ( x < 0.5 )
		return 4.0 * x - 0.5;
	else
		return -4.0 * x + 3.5;
}

float colormap_blue ( float x ) {
	if ( x < 0.3 )
		return 4.0 * x + 0.5;
	else
		return -4.0 * x + 2.5;
}

vec3 jet ( float x ) {
	float r = clamp ( colormap_red ( x ), 0.0, 1.0 );
	float g = clamp ( colormap_green ( x ), 0.0, 1.0 );
	float b = clamp ( colormap_blue ( x ), 0.0, 1.0 );
	return vec3 ( r, g, b );
}

void main() {

	float intensity = ( frag_color.x + frag_color.y + frag_color.z );
	if ( blend_mode == 1 ) { // GL_ONE, GL_ONE
		// out_color = vec4 ( frag_color * alpha, intensity );
		out_color = vec4 ( frag_color * alpha, intensity ); //hueGradient(float t)
	}
	else
		out_color = vec4 ( jet ( frag_color.r ), alpha );
		
}
