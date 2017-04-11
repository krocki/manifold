/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-11 10:15:52
*/

#ifndef __PLOTDATA_H__
#define __PLOTDATA_H__

#include <utils.h>
#include <random>

#include <gl/tex.h>
#include <io/import.h>
#include <iostream>

typedef enum matrix_layout { FLAT = 0, SQUARES = 1 } matrix_layout;

class PlotData {

  public:

	PlotData() {};
	~PlotData() {};

	Eigen::MatrixXf e_vertices, e_colors;
	Eigen::MatrixXf r_vertices;
	Eigen::MatrixXf p_vertices, p_colors;
	Eigen::MatrixXf s_vertices, s_colors;

	Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> c_indices;
	Eigen::MatrixXf c_vertices, c_colors;

	Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> m_indices;
	Eigen::MatrixXf m_vertices, m_colors, m_texcoords;

	// using imagesDataType = std::vector<std::pair<int, std::string>>;
	// imagesDataType textures;

	Texture input_data_textures;
	Texture input_reconstruction_textures;
	Texture sample_reconstruction_textures;

	std::vector<std::pair<int, std::string>> icons;

	//TODO: change to dict
	std::vector<Texture> nn_matrix_data_x;
	std::vector<Texture> nn_matrix_data_y;
	std::vector<Texture> nn_weight_data;
	std::vector<Texture> nn_matrix_data_dx;
	std::vector<Texture> nn_matrix_data_dy;
	std::vector<Texture> nn_weight_data_dW;

	std::vector<std::vector<std::pair<int, std::string>>> nn_matrices;

	NVGcontext *nvg;

	void updated() { checksum++; }

	size_t checksum = 0; // or write update time

	void load_input_data_textures ( std::deque<datapoint> &data, NVGcontext *nvg, size_t im_size, bool rgba = false ) {

		if ( rgba ) {
			input_data_textures = Texture ( data, GL_RGBA, nvg, im_size );
		} else {
			input_data_textures = Texture ( data, GL_RED, nvg, im_size );
		}

	}

	void update_reconstructions(std::deque<datapoint> &data, std::deque<datapoint> &sample_data, NVGcontext *nvg, size_t im_size, bool rgba = false ) {

		if ( rgba ) {
			input_reconstruction_textures = Texture ( data, GL_RGBA, nvg, im_size );
			sample_reconstruction_textures = Texture ( sample_data, GL_RGBA, nvg, im_size );
		} else {
			input_reconstruction_textures = Texture ( data, GL_RED, nvg, im_size );
			sample_reconstruction_textures = Texture ( sample_data, GL_RED, nvg, im_size );
		}
	}

	void update_nn_matrix_textures ( std::shared_ptr<NN> &net, NVGcontext *nvg, GLint fmt ) {

		if ( net ) {

			nn_matrix_data_x.resize ( net->layers.size() );
			nn_matrix_data_y.resize ( net->layers.size() );
			nn_weight_data.resize ( net->layers.size() );
			nn_matrix_data_dx.resize ( net->layers.size() );
			nn_matrix_data_dy.resize ( net->layers.size() );
			nn_weight_data_dW.resize ( net->layers.size() );

			nn_matrices.resize(net->layers.size());



			for ( size_t i = 0; i < net->layers.size(); i++ )
				if ( net->layers[i] ) {

					make_textures_from_matrices ( nn_matrix_data_x[i], net->layers[i]->x, nvg, fmt, SQUARES );
					make_textures_from_matrices ( nn_matrix_data_y[i], net->layers[i]->y, nvg, fmt, SQUARES );
					make_textures_from_matrices ( nn_matrix_data_dx[i], net->layers[i]->dx, nvg, fmt, SQUARES );
					make_textures_from_matrices ( nn_matrix_data_dy[i], net->layers[i]->dy, nvg, fmt, SQUARES );

					nn_matrices[i].clear();
					nn_matrices[i].push_back(std::make_pair(nn_matrix_data_x[i].id, "x"));
					nn_matrices[i].push_back(std::make_pair(nn_matrix_data_dx[i].id, "x"));
					nn_matrices[i].push_back(std::make_pair(nn_matrix_data_y[i].id, "y"));
					nn_matrices[i].push_back(std::make_pair(nn_matrix_data_dy[i].id, "dy"));

					if (net->layers[i]->name == "linear") {
						make_textures_from_matrices ( nn_weight_data[i], ( ( Linear * ) net->layers[i] )->W, nvg, fmt, SQUARES, true );
						make_textures_from_matrices ( nn_weight_data_dW[i], ( ( Linear * ) net->layers[i] )->dW, nvg, fmt, SQUARES, true );
						nn_matrices[i].push_back(std::make_pair(nn_weight_data[i].id, "W"));
						nn_matrices[i].push_back(std::make_pair(nn_weight_data_dW[i].id, "dW"));

					}
				}

			// reconstructions - output from the top layer
			// if ( net->layers.back() ) {
			// 	make_textures_from_matrices ( nn_matrix_data.back(), net->layers.back()->y, nvg, fmt, SQUARES );
			// }

		} else

			std::cout << "net is null" << std::endl;
	}

	void make_textures_from_matrices ( Texture &t, Eigen::MatrixXf &m, NVGcontext *nvg, GLint fmt = GL_RED,
	                                   matrix_layout layout = SQUARES, bool transpose = false ) {

		if ( m.size() > 0 ) {
			std::cout << "make_textures_from_matrices start" << std::endl;

			if ( layout == SQUARES ) {

				std::cout << "loading tex" << std::endl;
				// m is (vec length x batch size)
				if ( fmt == GL_RGBA )
					t.update_from_matrix ( m, fmt, nvg, transpose ? sqrt ( m.cols() / 3 ) : sqrt ( m.rows() / 3 ), transpose );
				else
					t.update_from_matrix ( m, fmt, nvg, transpose ? sqrt ( m.cols() ) : sqrt ( m.rows() ), transpose );

			} else {

				// flat
			}

			std::cout << "make_textures_from_matrices end" << std::endl;
		}
	}

};

void generate_cube ( Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> &indices, Eigen::MatrixXf &positions,
                     Eigen::MatrixXf &colors, Eigen::Vector3f t, float r = 1.0f, Eigen::Vector3f color = Eigen::Vector3f ( 0.2f, 0.2f,
                             0.2f ) ) {

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

#include <iostream>

void generate_mesh ( Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> &indices, Eigen::MatrixXf &positions,
                     Eigen::MatrixXf &texcoords, Eigen::MatrixXf &colors, Eigen::Vector3f t, float r = 1.0f,
                     Eigen::Vector3f color = Eigen::Vector3f ( 1.0f, 1.0f, 1.0f ) ) {

	int i = 1;
	int j = 1;

	indices.resize ( 3, i * j * 2 );
	positions.resize ( 3, ( j + 1 ) * ( i + 1 ) );
	colors.resize ( 3, ( j + 1 ) * ( i + 1 ) );
	texcoords.resize ( 3, ( j + 1 ) * ( i + 1 ) );

	for ( int l = 0; l < j + 1; l++ ) {
		for ( int k = 0; k < i + 1; k++ ) {

			positions.col ( k + ( i + 1 ) * l ) << r *k,  r *l,  0;
			texcoords.col ( k + ( i + 1 ) * l ) << k,  l,  0;

		}
	}

	for ( int l = 0; l < j; l++ ) {
		for ( int k = 0; k < i; k++ ) {

			indices.col ( k + l * i ) << k + ( i + 1 ) * l , k + ( i + 1 ) * l + 1 , k + ( i + 1 ) * ( l + 1 );
			indices.col ( i * j + k + l * i ) << k + ( i + 1 ) * l + 1 , k + ( i + 1 ) * ( l + 1 ) + 1 , k + ( i + 1 ) * ( l + 1 );
		}
	}
	// translation

	positions.colwise() -= Eigen::Vector3f ( i * r / 2, j * r / 2, 0 );
	positions.colwise() += t;

	for ( int l = 0; l < j + 1; l++ ) {
		for ( int k = 0; k < i + 1; k++ )

			colors.col ( k + ( i + 1 ) * l ) = color;

	}
};

#endif
