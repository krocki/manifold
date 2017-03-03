/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 14:00:12
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 14:15:02
*/

// to surpress warnings
#define UNUSED(...) [__VA_ARGS__](){};

// FPS
#define FPS_HISTORY_SIZE 50

double last_time = glfwGetTime();
int num_frames = 0;

std::vector<float> FPS ( FPS_HISTORY_SIZE, 0 );

void update_FPS ( void ) {

	double current_time = glfwGetTime();
	num_frames++;

	double t = current_time - last_time;

	float interval = 0.25f;

	if ( t >= interval ) {

		FPS.push_back ( ( num_frames / t ) );
		FPS.erase ( FPS.begin() );

		num_frames = 0;
		last_time += interval;

	}
}

// rands
float rand_float ( float mn, float mx ) {

	float r = random() / ( float ) RAND_MAX;
	return mn + ( mx - mn ) * r;
}

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