#version 330

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

uniform vec3 offset;
uniform mat4 perspective;
uniform vec3 angle;

layout ( location = 0 ) in vec3 vertex_in_position;
in vec3 vertex_in_color;
smooth out vec4 fragment_in_color;

void main() {

	mat4 xRMatrix = mat4 ( cos ( angle.x ), 0.0, sin ( angle.x ), 0.0,
						   0.0, 1.0, 0.0, 0.0,
						   -sin ( angle.x ), 0.0, cos ( angle.x ), 0.0,
						   0.0, 0.0, 0.0, 1.0 );
						   
	mat4 yRMatrix = mat4 ( 1.0, 0.0, 0.0, 0.0,
						   0.0, cos ( angle.y ), -sin ( angle.y ), 0.0,
						   0.0, sin ( angle.y ), cos ( angle.y ), 0.0,
						   0.0, 0.0, 0.0, 1.0 );
						   
	mat4 zRMatrix = mat4 ( cos ( angle.z ), -sin ( angle.z ), 0.0, 0.0,
						   sin ( angle.z ) , cos ( angle.z ), 0.0, 0.0,
						   0.0, 0.0, 1.0, 0.0,
						   0.0, 0.0, 0.0, 1.0 );
						   
	vec4 rotatedPosition = vec4 ( vertex_in_position.xyz, 1.0f ) * zRMatrix * xRMatrix * yRMatrix;
	vec4 cameraPos = rotatedPosition + vec4 ( offset.x, offset.y, offset.z, 0.0 );
	
	gl_Position = perspective * cameraPos;
	fragment_in_color = vec4 ( vertex_in_color, 1.0 );
	// fragment_in_color = mix( vec4( vertex_in_color.x, vertex_in_color.y, vertex_in_color.z, 1.0 ),
	//                          vec4( 0.0f, vertex_in_color.y, vertex_in_color.z, 1.0 ), vertex_in_position.y / 10 );
	
}