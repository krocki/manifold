/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-04-06 15:45:41
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
		// Eigen::VectorXf p_labels;
		// Eigen::MatrixXf p_texcoords;
		
		Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> c_indices;
		Eigen::MatrixXf c_vertices, c_colors;
		
		Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> m_indices;
		Eigen::MatrixXf m_vertices, m_colors, m_texcoords;
		
		// using imagesDataType = std::vector<std::pair<int, std::string>>;
		// imagesDataType textures;
		
		Texture c100_data_textures;
		
		//TODO: change to dict
		std::vector<Texture> nn_matrix_data;
		std::vector<Texture> nn_weight_data;
		
		void updated() { checksum++; }
		
		size_t checksum = 0; // or write update time
		
		void load_c100_data_textures ( std::deque<datapoint> &data, NVGcontext *nvg, size_t im_size, bool rgba = false ) {
		
			if ( rgba )
				c100_data_textures = Texture ( data, GL_RGBA, nvg, im_size );
			else
				c100_data_textures = Texture ( data, GL_RED, nvg, im_size );
				
		}
		
		void update_nn_matrix_textures ( std::shared_ptr<NN> &net, NVGcontext *nvg, GLint fmt ) {
		
			if ( net ) {
			
				nn_matrix_data.resize ( net->layers.size() );
				nn_weight_data.resize ( net->layers.size() );
				
				// for ( size_t i = 0; i < net.layers.size(); i++ ) {
				
				
				// }
				
				
				if ( net->layers[0] ) {
					// inputs to the lowermost layer
					make_textures_from_matrices ( nn_matrix_data[0], net->layers[0]->x, nvg, fmt, SQUARES );
					// weights layer 0
					make_textures_from_matrices ( nn_weight_data[0], ( ( Linear * ) net->layers[0] )->W, nvg, fmt, SQUARES, true );
				}
				
				// reconstructions - output from the top layer
				if ( net->layers.back() )
					make_textures_from_matrices ( nn_matrix_data.back(), net->layers.back()->y, nvg, fmt, SQUARES );
			}
			else
			
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
						
				}
				else {
				
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
