/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 14:00:12
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-21 13:22:32
*/

#ifndef _AUX_H_
#define _AUX_H_

// functions
float hat ( float x, float y ) {

	float t = hypotf ( x, y ) * 4.0;
	float z = ( 1 - t * t ) * expf ( t * t / -2.0 );
	return z;

}

//colormaps
float hue2rgb ( float p, float q, float t ) {

	float tt = t;
	if ( tt < 0.0 ) tt += 1.0;
	if ( tt > 1.0 ) tt -= 1.0;
	if ( tt < 1.0 / 6.0 ) return p + ( q - p ) * 6.0 * tt;
	if ( tt < 1.0 / 2.0 ) return q;
	if ( tt < 2.0 / 3.0 ) return p + ( q - p ) * ( 2.0 / 3.0 - tt ) * 6.0;
	return p;

}

Eigen::Vector3f hslToRgb ( float h, float s, float l ) {
	float r, g, b;
	if ( s == 0.0 ) {
		r = g = b = l; // achromatic
	} else {
		float q;
		if ( l < 0.5 )
			q = l * ( 1.0 + s );
		else
			q = l + s - l * s;

		float p = 2.0 * l - q;
		r = hue2rgb ( p, q, h + 1.0 / 3.0 );
		g = hue2rgb ( p, q, h );
		b = hue2rgb ( p, q, h - 1.0 / 3.0 );
	}

	return Eigen::Vector3f ( r, g, b );
}

/*NVGcolor nvgRGB(unsigned char r, unsigned char g, unsigned char b);

// Returns a color value from red, green, blue values. Alpha will be set to 1.0f.
NVGcolor nvgRGBf(float r, float g, float b);


// Returns a color value from red, green, blue and alpha values.
NVGcolor nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// Returns a color value from red, green, blue and alpha values.
NVGcolor nvgRGBAf(float r, float g, float b, float a);


// Linearly interpolates from color c0 to c1, and returns resulting color value.
NVGcolor nvgLerpRGBA(NVGcolor c0, NVGcolor c1, float u);

// Sets transparency of a color value.
NVGcolor nvgTransRGBA(NVGcolor c0, unsigned char a);

// Sets transparency of a color value.
NVGcolor nvgTransRGBAf(NVGcolor c0, float a);

// Returns color value specified by hue, saturation and lightness.
// HSL values are all in range [0..1], alpha will be set to 255.
NVGcolor nvgHSL(float h, float s, float l);

// Returns color value specified by hue, saturation and lightness and alpha.
// HSL values are all in range [0..1], alpha in range [0..255]
NVGcolor nvgHSLA(float h, float s, float l, unsigned char a);
*/
#endif