#version 330

uniform mat4 mvp;
in vec3 position;
in vec3 color;
out vec3 frag_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-06 11:10:30
*/

/* TODO - not sure if this works */

void main() {

	vec4 tmpPoint = mvp * vec4 ( position, 1.0 );
	tmpPoint.xy = tmpPoint.xy /  length ( tmpPoint.xyz );
	gl_Position = tmpPoint;
	
	if ( isnan ( position.r ) ) /* nan (missing value) */
		frag_color = vec3 ( 0.0 );
	else
		frag_color = color;
}
