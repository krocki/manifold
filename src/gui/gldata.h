/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-20 17:35:11
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

void generate_rand_points(Eigen::MatrixXf& vertices, Eigen::MatrixXf& colors, size_t N) {

	vertices.resize ( 3, N );
	colors.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		vertices.col ( i ) << rand_float(-0.25f, 0.25f), rand_float(-0.25f, 0.25f), rand_float(-0.25f, 0.25f);
		colors.col ( i ) = Eigen::Vector3f(0.0f, 1.0f, 0.0f);

	}

}

void generate_randn_points(Eigen::MatrixXf& vertices, Eigen::MatrixXf& colors, size_t N, float mean = 0.0f, float stddev = 1.0f) {

	vertices.resize ( 3, N );
	colors.resize ( 3, N );

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::normal_distribution<> randn ( mean, stddev );

	for ( size_t i = 0; i < N; i++ ) {

		vertices.col ( i ) << randn ( mt ), randn ( mt ), randn ( mt );
		colors.col ( i ) = Eigen::Vector3f(0.0f, 1.0f, 0.0f);

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