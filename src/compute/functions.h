#ifndef ___FUNCTIONS_H___
#define ___FUNCTIONS_H___

#include <utils.h>

std::random_device rd;
std::mt19937 mt ( rd() );
std::normal_distribution<> randn ( 0.0f, 1.0f );

// std::uniform_int_distribution<> randi ( -100.0f, 100.0f );
// float discrete(float scale, size_t i = 0) {

// 	UNUSED(i);
// 	return randi ( mt ) / (100.0f / scale);

// }

namespace func1 {

float normal(float scale, size_t i = 0) {

	UNUSED(i);
	return scale * randn ( mt );

}

float uniform(float scale, size_t i = 0) {

	UNUSED(i);
	return rand_float(-scale, scale);

}

}
namespace func3 {

float hat ( float x, float y ) {

	float t = hypotf ( x, y ) * 1.0;
	float z = ( 1 - t * t ) * expf ( t * t / -2.0 );
	return z;

}

}

template <class Function, class Sampler>
void generate (Function f, Sampler s, Eigen::MatrixXf& points, size_t N = 1, float scale = 1.0f) {

	points.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		float x = s(scale, i);
		float y = s(scale, i);

		points.col ( i ) << x, y, scale * f(x, y);

	}
}

void set ( const Eigen::Vector3f c, Eigen::MatrixXf& points, size_t N = 1) {

	points.resize ( 3, N );
	points.colwise() = c;

}

#endif