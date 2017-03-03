#version 410

/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-03 11:10:30
*/

smooth in vec4 fragment_in_color;
out vec4 fragment_out_color;

void main() {
	fragment_out_color = fragment_in_color;
}