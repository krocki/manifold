#version 330

uniform sampler2D image;
out vec4 color;
in vec2 uv;

uniform int apply_color;
uniform int colormap;
uniform int discretize;
uniform float inv_alpha;

void main() {

	color = texture ( image, uv );
	color.a = 1.0f - inv_alpha;
	
	if ( apply_color == 1 )
		color = apply_colormap ( color.rgb, colormap, 1.0f - inv_alpha, discretize );
		
}