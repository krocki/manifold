/*
* @Author: kmrocki
* @Date:   2016-02-24 10:47:03
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-13 13:52:15
*/

#ifndef __NN_UTILS_H__
#define __NN_UTILS_H__

#include <utils.h>

//set Matrix & Vector implementation
#include <Eigen/Dense>

#define dtype float
typedef Eigen::MatrixXf Matrix;
typedef Eigen::VectorXf Vector;

#include <io/import.h>
#include <iostream>
#include <random>

#include "perf.h"
#include "aux.h"

inline float sqrt_eps ( const float x ) {
	return sqrtf ( x + 1e-6f );
}

//f(x) = sigm(x)
inline float __logistic ( const float x ) {
	return 1.0f / ( 1.0f +::expf ( -x ) );
}

inline float __exponential ( const float x ) {
	return expf ( x );
}

#ifdef USE_BLAS
#include <cblas.h>
void BLAS_mmul ( Eigen::MatrixXf &__restrict c, Eigen::MatrixXf &__restrict a,
                 Eigen::MatrixXf &__restrict b, bool aT = false, bool bT = false );

#endif

void random_binary_mask ( Matrix &mask ) {

	for ( int i = 0; i < mask.rows(); i++ ) {
		for ( int j = 0; j < mask.cols(); j++ )

			mask ( i, j ) = round ( rand_float ( 0, 1 ) );

	}
}


Matrix rectify ( Matrix &x ) {

	Matrix y ( x.rows(), x.cols() );

	for ( int i = 0; i < x.rows(); i++ ) {
		for ( int j = 0; j < x.cols(); j++ )

			y ( i, j ) = x ( i, j ) > 0.0f ? x ( i, j ) : 0.0f;
	}

	return y;

}

// Exponential Linear Unit
// http://arxiv.org/pdf/1511.07289v5.pdf
Matrix activation_ELU ( Matrix &x ) {

	float alpha = 1.0f;

	Matrix y ( x.rows(), x.cols() );

	for ( int i = 0; i < x.rows(); i++ ) {
		for ( int j = 0; j < x.cols(); j++ )

			y ( i, j ) = x ( i, j ) >= 0.0f ? x ( i, j ) : alpha * ( expf ( x ( i, j ) ) - 1.0f );

	}

	return y;

}

Matrix derivative_ELU ( Matrix &x ) {

	float alpha = 1.0f;

	Matrix y ( x.rows(), x.cols() );

	for ( int i = 0; i < x.rows(); i++ ) {
		for ( int j = 0; j < x.cols(); j++ )

			y ( i, j ) = x ( i, j ) >= 0.0f ? 1.0f : ( x ( i, j ) + alpha );
	}

	return y;

}

Matrix derivative_ReLU ( Matrix &x ) {

	Matrix y ( x.rows(), x.cols() );

	for ( int i = 0; i < x.rows(); i++ ) {
		for ( int j = 0; j < x.cols(); j++ )

			y ( i, j ) = ( float ) ( x ( i, j ) > 0 );
	}

	return y;

}

Matrix logistic ( Matrix &x ) {

	Matrix y ( x.rows(), x.cols() );

	for ( int i = 0; i < x.rows(); i++ ) {
		for ( int j = 0; j < x.cols(); j++ )

			y ( i, j ) = __logistic ( x ( i, j ) );
	}

	return y;
}

Matrix softmax ( Matrix &x ) {

	Matrix y ( x.rows(), x.cols() );

	//probs(class) = exp(x, class)/sum(exp(x, class))

	Matrix e = x.unaryExpr ( std::ptr_fun ( ::expf ) );

	Vector sum = e.colwise().sum();

	for ( int i = 0; i < e.rows(); i++ ) {
		for ( int j = 0; j < e.cols(); j++ )

			y ( i, j ) = e ( i, j ) / sum ( j );
	}

	return y;
}

float cross_entropy ( Matrix &predictions, Matrix &targets ) {

	float ce = 0.0f;
	Matrix error ( predictions.rows(), predictions.cols() );

	//check what has happened and get information content for that event
	error.array() = -predictions.unaryExpr ( std::ptr_fun ( ::logf ) ).array() * targets.array();
	ce = error.sum();

	return ce;
}

float cross_entropy_mask ( Matrix &predictions, Matrix &targets, Matrix &mask, bool invert = false ) {

	float ce = 0.0f;
	Matrix error ( predictions.rows(), predictions.cols() );

	if (!invert)
		error.array() = -predictions.unaryExpr ( std::ptr_fun ( ::logf ) ).array() * mask.array();
	else
		error.array() = -(1.0f - predictions.array()).unaryExpr ( std::ptr_fun ( ::logf ) ).array() * mask.array();

	ce = error.sum();

	return ce;
}

float mse ( Matrix &yhat, Matrix &y ) {

	float mse = 0.0;
	Matrix error = y - yhat;

	//check what has happened and get information content for that event
	error.array() = 2.0f * error.array() * error.array();
	mse = error.sum();

	return mse;
}

//generate an array of random numbers in range
void matrix_randi ( Eigen::VectorXi &m, int range_min, int range_max ) {

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::uniform_int_distribution<> dis ( range_min, range_max );

	for ( int i = 0; i < m.rows(); i++ )
		m ( i ) = ( float ) dis ( mt );

}

void matrix_randi ( Eigen::MatrixXi &m, int range_min, int range_max ) {

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::uniform_int_distribution<> dis ( range_min, range_max );

	for ( int i = 0; i < m.size(); i++ )
		m ( i ) = ( float ) dis ( mt );

}

//generate an array of random numbers in range
void matrix_rand ( Matrix &m, float range_min, float range_max ) {

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::uniform_real_distribution<> randf ( range_min, range_max );

	for ( int i = 0; i < m.rows(); i++ ) {
		for ( int j = 0; j < m.cols(); j++ )
			m ( i, j ) = randf ( mt );

	}

}

void matrix_randn ( Matrix &m, float mean, float stddev ) {

	std::random_device rd;
	std::mt19937 mt ( rd() );
	std::normal_distribution<> randn ( mean, stddev );

	for ( int i = 0; i < m.rows(); i++ ) {
		for ( int j = 0; j < m.cols(); j++ )
			m ( i, j ) = randn ( mt );
	}

}

void linspace ( Eigen::VectorXi &m, int range_min, int range_max ) {

	// not really linspace - fixed increment = 1, TODO - fix, use range_max
	UNUSED ( range_max );

	for ( int i = 0; i < m.rows(); i++ )
		m ( i ) = ( float ) ( range_min + i );

}

void make_batch ( Matrix &batch, const Eigen::MatrixXf &data, const Eigen::VectorXi &random_numbers ) {

	// TODO: this crashes when batch_size is > than data[], not checking the bounds, for example , batch_size = 16, data = 5000, 5000 % 16 != 0, will access 5001 and crash
	size_t batch_size = random_numbers.rows();

	for ( size_t i = 0; i < batch_size; i++ )

		batch.col ( i ) = data.col ( random_numbers ( i ) );


}

void mix ( Eigen::MatrixXf &a, const Eigen::MatrixXf &b, const Eigen::MatrixXi &random_numbers ) {

	for ( size_t i = 0; i < random_numbers.size(); i++ )
		if ( random_numbers ( i ) == 1 ) a.col ( i ) = b.col ( i );

}

void mix ( Eigen::MatrixXf &a, const Eigen::MatrixXf &b, const Eigen::VectorXi &random_numbers ) {

	for ( size_t i = 0; i < random_numbers.rows(); i++ )
		if ( random_numbers ( i ) == 1 ) a.col ( i ) = b.col ( i );

}

void mix ( Eigen::VectorXf &a, const Eigen::VectorXf &b, const Eigen::VectorXi &random_numbers ) {

	for ( size_t i = 0; i < random_numbers.rows(); i++ )
		if ( random_numbers ( i ) == 1 ) a.col ( i ) = b.col ( i );

}

void make_batch ( Matrix &batch, const std::deque<datapoint> &data, const Eigen::VectorXi &random_numbers ) {

	// TODO: this crashes when batch_size is > than data[], not checking the bounds, for example , batch_size = 16, data = 5000, 5000 % 16 != 0, will access 5001 and crash
	size_t batch_size = random_numbers.rows();

	for ( size_t i = 0; i < batch_size; i++ )

		batch.col ( i ) = data[random_numbers ( i )].x;


}

void make_batch ( Matrix &batch, const std::deque<datapoint> &data, const Eigen::MatrixXi &random_numbers ) {

	// TODO: this crashes when batch_size is > than data[], not checking the bounds, for example , batch_size = 16, data = 5000, 5000 % 16 != 0, will access 5001 and crash
	size_t batch_size = random_numbers.cols();

	for ( size_t i = 0; i < batch_size; i++ )

		batch.col ( i ) = data[random_numbers(0, i )].x;


}

void make_targets ( Matrix &targets, const Matrix &encoding, const std::deque<datapoint> &data,
                    Eigen::VectorXi &random_numbers ) {

	size_t batch_size = random_numbers.rows();

	for ( size_t i = 0; i < ( size_t ) batch_size; i++ )

		targets.col ( i ) = encoding.col ( data[random_numbers ( i )].y );


}

Eigen::VectorXi colwise_max_index ( Matrix &m ) {

	Eigen::VectorXi indices ( m.cols() );

	for ( size_t i = 0; i < ( size_t ) m.cols(); i++ ) {

		float current_max_val;
		int index;

		for ( size_t j = 0; j < ( size_t ) m.rows(); j++ ) {

			if ( j == 0 || m ( j, i ) > current_max_val ) {

				index = j;
				current_max_val = m ( j, i );
			}

			indices ( i ) = index;

		}
	}

	return indices;
}

size_t count_zeros ( Eigen::VectorXi &m ) {

	size_t zeros = 0;

	for ( int i = 0; i < m.rows(); i++ ) {

		bool isZero = m ( i ) == 0;

		zeros += isZero;
	}

	return zeros;

}

size_t count_correct_predictions ( Matrix &p, Matrix &t ) {

	Eigen::VectorXi predicted_classes = colwise_max_index ( p );
	Eigen::VectorXi target_classes = colwise_max_index ( t );
	Eigen::VectorXi correct = ( target_classes - predicted_classes );

	return count_zeros ( correct );
}

#ifdef USE_BLAS
// c = a * b
void BLAS_mmul ( Eigen::MatrixXf &__restrict c, Eigen::MatrixXf &__restrict a,
                 Eigen::MatrixXf &__restrict b, bool aT, bool bT ) {

	enum CBLAS_TRANSPOSE transA = aT ? CblasTrans : CblasNoTrans;
	enum CBLAS_TRANSPOSE transB = bT ? CblasTrans : CblasNoTrans;

	size_t M = c.rows();
	size_t N = c.cols();
	size_t K = aT ? a.rows() : a.cols();

	float alpha = 1.0f;
	float beta = 1.0f;

	size_t lda = aT ? K : M;
	size_t ldb = bT ? N : K;
	size_t ldc = M;

	cblas_sgemm ( CblasColMajor, transA, transB, M, N, K, alpha,
	              a.data(), lda,
	              b.data(), ldb, beta, c.data(), ldc );

	flops_performed += 2 * M * N * K;
	bytes_read += ( a.size() + b.size() ) * sizeof ( dtype );
}
#endif /* USE_BLAS */

#endif