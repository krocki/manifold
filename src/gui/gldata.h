/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-20 20:55:44
*/

#ifndef __PLOTDATA_H__
#define __PLOTDATA_H__

#include <utils.h>
#include <random>

class PlotData {

  public:

	PlotData() {};
	~PlotData() {};

	Eigen::MatrixXf p_vertices, p_colors;
	Eigen::MatrixXf c_vertices, c_colors;

	void updated() { checksum++; }

	size_t checksum = 0; // or write update time

};

void generate_rand_points(Eigen::MatrixXf& points, size_t N, float minval = 0.0f, float maxval = 1.0f) {

	points.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		points.col ( i ) << rand_float(minval, maxval), rand_float(minval, maxval), rand_float(minval, maxval);

	}

}

void generate_randn_points(Eigen::MatrixXf& points, size_t N, float mean = 0.0f, float stddev = 1.0f) {

	points.resize ( 3, N );

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::normal_distribution<> randn ( mean, stddev );

	for ( size_t i = 0; i < N; i++ ) {

		points.col ( i ) << randn ( mt ), randn ( mt ), randn ( mt );

	}

}

void generate_cube(Eigen::MatrixXf& vertices, Eigen::MatrixXf& colors, Eigen::Vector3f t, float r = 1.0f, Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f)) {

	vertices.resize ( 3, 36 );
	colors.resize ( 3, 36 );

	vertices <<
	         -r,  r, -r, -r, -r, -r,  r, -r, -r,  r, -r, -r,  r,  r, -r, -r,  r, -r,
	         -r, -r,  r, -r, -r, -r, -r,  r, -r, -r,  r, -r, -r,  r,  r, -r, -r,  r,
	         r, -r, -r,  r, -r,  r,  r,  r,  r,  r,  r,  r,  r,  r, -r,  r, -r, -r,
	         -r, -r,  r, -r,  r,  r,  r,  r,  r,  r,  r,  r,  r, -r,  r, -r, -r,  r,
	         -r,  r, -r,  r,  r, -r,  r,  r,  r,  r,  r,  r, -r,  r,  r, -r,  r, -r,
	         -r, -r, -r, -r, -r,  r,  r, -r, -r,  r, -r, -r, -r, -r,  r,  r, -r,  r;

	// translation
	vertices.colwise() += t;

	colors.colwise() = color;
};

#endif