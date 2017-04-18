#version 330

uniform mat4 mvp;
in vec3 position;
in vec3 color;
out vec3 frag_color;

uniform int coord_type;
uniform float pt_size;
uniform float alpha;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

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
		
	gl_Position = mvp * vec4 ( coords, 1.0 );
	gl_PointSize = pt_size;
	frag_color = color;
}
