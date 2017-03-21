/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-21 01:36:21
*/

#ifndef __PLOTDATA_H__
#define __PLOTDATA_H__

#include <utils.h>
#include <random>

class PlotData {

public:

	PlotData() {};
	~PlotData() {};

	Eigen::MatrixXf e_vertices, e_colors;
	Eigen::MatrixXf p_vertices, p_colors;

	Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> c_indices;
	Eigen::MatrixXf c_vertices, c_colors;

	void updated() { checksum++; }

	size_t checksum = 0; // or write update time

};

void generate_points(Eigen::MatrixXf& points, size_t N, const Eigen::Vector3f c) {

	points.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		points.col ( i ) = c;

	}

}

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

void generate_cube(Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>& indices, Eigen::MatrixXf& positions, Eigen::MatrixXf& colors, Eigen::Vector3f t, float r = 1.0f, Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f)) {

	indices.resize ( 2, 12 );
	positions.resize ( 3, 8 );
	colors.resize ( 3, 12 );

	indices.col ( 0 ) << 0, 1;
	indices.col ( 1 ) << 1, 2;
	indices.col ( 2 ) << 2, 3;
	indices.col ( 3 ) << 3, 0;
	indices.col ( 4 ) << 4, 5;
	indices.col ( 5 ) << 5, 6;
	indices.col ( 6 ) << 6, 7;
	indices.col ( 7 ) << 7, 4;
	indices.col ( 8 ) << 0, 4;
	indices.col ( 9 ) << 1, 5;
	indices.col ( 10 ) << 2, 6;
	indices.col ( 11 ) << 3, 7;

	positions.col ( 0 ) << -r,  r,  r;
	positions.col ( 1 ) << -r,  r, -r;
	positions.col ( 2 ) <<  r,  r, -r;
	positions.col ( 3 ) <<  r,  r,  r;
	positions.col ( 4 ) << -r, -r,  r;
	positions.col ( 5 ) << -r, -r, -r;
	positions.col ( 6 ) <<  r, -r, -r;
	positions.col ( 7 ) <<  r, -r,  r;

	// translation
	positions.colwise() += t;
	colors.colwise() = color;
};

#endif