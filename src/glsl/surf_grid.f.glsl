#version 330

out vec4 out_color;
smooth in vec4 fragment_in_color;

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

void main() {
	out_color = vec4 ( fragment_in_color );
}