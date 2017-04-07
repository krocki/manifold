#version 330

in vec3 position;
in vec3 color;
out vec3 out_v_color;

in vec3 texcoords;
out vec4 pos;
out vec2 uv;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

uniform int coord_type;
uniform mat4 model;
uniform mat4 view;

void main() {

	vec3 coords;
	
	if ( coord_type == 1 ) {
	
		//polar
		float r = position[0];
		float theta = position[1];
		coords = vec3 ( r * cos ( theta ), r * sin ( theta ), position[2] );
		
	}
	else
		if ( coord_type == 2 ) {
		
			//spherical
			float r = position[0];
			float theta = position[1];
			float phi = position[2];
			
			coords = vec3 ( r * sin ( phi ) * cos ( theta ), r * sin ( theta ) * sin ( phi ), r * cos ( phi ) );
			
		}
		else {
		
			//normal
			coords = position;
		}
		
	uv = texcoords.xy;
	
	out_v_color = vec3 ( color );
	gl_Position = view * model * vec4 ( coords, 1.0 );
	
}