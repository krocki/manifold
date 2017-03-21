/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 14:00:12
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-20 13:34:56
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

#endif