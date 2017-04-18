
/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-09 22:20:32
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-18 06:41:31
*/

#include <nanogui/colormaps.h>

#ifndef __COLORS_H__
#define __COLORS_H__

std::vector<std::string> available_colormaps = {"gray", "jet", "parula", "viridis", "hsv", "rgb", "haxby", "seismic"};

float interpolate_jet ( float val, float y0, float x0, float y1, float x1 ) {

	return ( val - x0 ) * ( y1 - y0 ) / ( x1 - x0 ) + y0;
}

float jet_base ( float val ) {

	if ( val <= -0.75f ) return 0;
	else if ( val <= -0.25f ) return interpolate_jet ( val, 0.0f, -0.75f, 1.0f, -0.25f );
	else if ( val <= 0.25f ) return 1.0f;
	else if ( val <= 0.75f ) return interpolate_jet ( val, 1.0f, 0.25f, 0.0f, 0.75f );
	else return 0.0f;

}

float jet_red ( float value ) {

	return jet_base ( value - 0.6f );
}

float jet_green ( float value ) {

	return jet_base ( value );
}

float jet_blue ( float value ) {

	return jet_base ( value + 0.6f );
}


nanogui::Color jet_colormap ( float value ) {

	value *= 2.0f;
	value -= 1.0f;

	//MATLAB's jet palette
	float r = jet_red ( value );
	float g = jet_green ( value );
	float b = jet_blue ( value );

	return nanogui::Color ( ( uint8_t ) ( r * 255.0f ), ( uint8_t ) ( g * 255.0f ), ( uint8_t ) ( b * 255.0f ), 127 );
}


nanogui::Color parula_colormap ( float value ) {

	//simple discretization, NN
	int index = round ( value * 10.0f - 0.5f );

	index = fmin ( index, nanogui::PARULA_LUT_SIZE - 1 );
	index = fmax ( index, 0 );

	return nanogui::parula_lut[index];

}

nanogui::Color bar_colormap ( float value ) {

	//simple discretization, NN
	int index = round ( value * 10.0f - 0.5f );

	index = fmin ( index, nanogui::BARCOLOR_LUT_SIZE - 1 );
	index = fmax ( index, 0 );

	return nanogui::barcolormap_lut[index];

}

// _float4 haxby_colormap(float value) {

//     _float4 color;

//     unsigned low = (unsigned)(float)floor(value * (haxby_colors - 1));
//     unsigned high = (unsigned)(float)ceil(value * (haxby_colors - 1));

//     float v = (float)(haxby_colors - 1) * (value - (float)floor(value * (haxby_colors - 1)) / (haxby_colors - 1));

//     float r = (float)linear_interpolation( v, haxby[low][0], haxby[high][0] );
//     float g = (float)linear_interpolation( v, haxby[low][1], haxby[high][1] );
//     float b = (float)linear_interpolation( v, haxby[low][2], haxby[high][2] );

//     color.r = r / 255.0f;
//     color.g = g / 255.0f;
//     color.b = b / 255.0f;
//     color.a = 1.0f;

//     return color;

// }

// _float4 seismic_colormap(float value) {

//     _float4 color;

//     unsigned low = (unsigned)(float)floor(value * (seismic_colors - 1));
//     unsigned high = (unsigned)(float)ceil(value * (seismic_colors - 1));

//     float v = (float)(seismic_colors - 1) * (value - (float)floor(value * (seismic_colors - 1)) / (seismic_colors - 1));

//     float r = (float)linear_interpolation( v, seismic[low][0], seismic[high][0] );
//     float g = (float)linear_interpolation( v, seismic[low][1], seismic[high][1] );
//     float b = (float)linear_interpolation( v, seismic[low][2], seismic[high][2] );

//     color.r = r / 255.0f;
//     color.g = g / 255.0f;
//     color.b = b / 255.0f;
//     color.a = 1.0f;

//     return color;

// }

#endif