/*
* @Author: kmrocki
* @Date:   2016-02-24 15:28:10
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-04-18 14:03:12
*/

#ifndef __GAN_NN_H__
#define __GAN_NN_H__

#include <io/import.h>
#include <nn/layers.h>
#include <nn/opt.h>
#include <unistd.h>
#include <colors.h>
#include <mutex>

#include "perf.h"

#include <nanogui/serializer/core.h>

// TODO:
// Sparse
// KL instead of rec
// cross entropy for AE
// VAE
// Normal noise
// Semantic hashing
// ladder network
// resnet/ highway

typedef enum network_type { MLP = 0, AE = 1, DAE = 2 } network_type;
typedef enum nonlinearity_type { RELU = 0, SIGMOID = 1, ELU = 2 } nonlinearity_type;

/* http://torch.ch/blog/2015/11/13/gan.html

A few other tricks are necessary for successful GAN training:

Batch normalization speeds up training a lot when used in the generator. Using batch normalization in the discriminator is dangerous as the discriminator becomes too powerful.

Plenty of dropout is needed in the discriminator to avoid oscillating behavior caused by the generator exploiting a weakness of the discriminator. Dropout can also be used in the generator.

It may be beneficial to limit the capacity of the discriminator. This is done by decreasing its number of features such that the generator contains more parameters.

*/

class NN {

	public:
	
		std::vector<int> layer_sizes;
		std::deque<Layer *> layers;
		float current_loss = -0.01f;
		Eigen::VectorXf *loss_data, *real_acc_data, *fake_acc_data, *w_norm_data, *dw_norm_data, *mw_norm_data;
		
		float learning_rate;
		
		bool use_code_sigmoid = false;
		bool use_dropout = false;
		bool collect_stats_enabled = true;
		
		bool clock = false;
		bool quit = false;
		bool pause = false;
		bool step = false;
		
		int batch_size;
		float decay;
		
		network_type ntype;
		nonlinearity_type ltype;
		opt_type otype;
		
		Eigen::VectorXi random_numbers;
		
		Matrix batch, sample_batch;
		Matrix targets;
		Matrix encoding;
		Matrix codes, codes_colors;
		Eigen::MatrixXi codes_idxs, sample_idx;
		
		float epoch_progress;
		
		int code_layer_no = 0;
		
		std::mutex params;
		
		void forward ( int min_layer, const Matrix &input_data ) {
		
			//copy inputs to the lowest point in the network
			layers[min_layer]->x = input_data;
			
			//compute forward activations
			for ( size_t i = min_layer; i < layers.size(); i++ ) {
			
				//y = f(x)
				layers[i]->forward();
				
				//x(next layer) = y(current layer)
				if ( i + 1 < layers.size() )
					layers[i + 1]->x = layers[i]->y;
					
			}
			
		}
		
		void forward ( const Matrix &input_data, int max_layer = -1 ) {
		
			//copy inputs to the lowest point in the network
			layers[0]->x = input_data;
			
			//compute forward activations
			for ( size_t i = 0; i < layers.size(); i++ ) {
			
				//y = f(x)
				layers[i]->forward();
				
				//x(next layer) = y(current layer)
				if ( i + 1 < layers.size() )
					layers[i + 1]->x = layers[i]->y;
					
				if ( max_layer != -1 && ( int ) i == max_layer ) break;
				
			}
			
		}
		
		void backward ( const Matrix &t ) {
		
			//set targets at the top
			layers[layers.size() - 1]->dy = t;
			
			//propagate error backward
			for ( int i = layers.size() - 1; i >= 0; i-- ) {
			
				layers[i]->resetGrads();
				
				// sparsify dy grads
				//if ( i == 0 ) layers[i]->sparsify();
				// if ( i == ( layers.size() - 2 ) ) layers[i]->sparsify();
				
				layers[i]->backward();
				
				//dy(previous layer) = dx(current layer)
				if ( i > 0 )
					layers[i - 1]->dy = layers[i]->dx;
					
			}
			
		}
		
		void update ( double alpha, float decay = 0.0f ) {
		
			params.lock();
			//update all layers according to gradients
			for ( size_t i = 0; i < layers.size(); i++ )
			
				layers[i]->applyGrads ( alpha, decay );
				
				
			params.unlock();
			
		}
		
		void collect_statistics ( ) {
		
			for ( size_t l = 0; l < layers.size(); l++ )
				layers[l]->collect_statistics ( );
		}
		
		float get_total_w_norm ( ) {
		
		
			float s = 0.0f;
			for ( size_t l = 0; l < layers.size(); l++ )
				s += layers[l]->w_norm();
				
			return s;
			
		}
		
		float get_total_dw_norm ( ) {
		
		
			float s = 0.0f;
			for ( size_t l = 0; l < layers.size(); l++ )
				s += layers[l]->dw_norm();
				
			return s;
			
		}
		
		float get_total_mw_norm ( ) {
		
		
			float s = 0.0f;
			for ( size_t l = 0; l < layers.size(); l++ )
				s += layers[l]->mw_norm();
				
			return s;
			
		}
		
		void train ( const Eigen::MatrixXf &s_vertices, std::deque<datapoint> &reconstruction_data ) {
		
			while ( pause ) {
				usleep ( 10000 );
				if ( step || quit ) { step = false; break; }
			}
			
			if ( !quit ) {
			
				size_t dims = layers[code_layer_no]->x.rows();
				
				sample_idx.resize ( 1, s_vertices.cols() );
				reconstruction_data.resize ( s_vertices.cols() );
				
				Eigen::VectorXi numbers ( batch_size );
				
				sample_batch.resize ( dims, batch_size );
				
				size_t iterations = s_vertices.cols() / batch_size;
				
				for ( size_t ii = 0; ii < s_vertices.cols(); ii += batch_size ) {
				
					linspace ( numbers, ii, ii + batch_size );
					make_batch ( sample_batch, s_vertices, numbers );
					
					ticf();
					tic();
					
					forward ( code_layer_no, sample_batch );
					
					tocf();
					toc();
					
					if ( ii % 250 == 0 ) {
					
						clock = true;
						
						if ( collect_stats_enabled )
							collect_statistics();
					}
					
					epoch_progress = ( ( float ) ii ) / ( float ) iterations;
					
					if ( quit ) break;
				}
				
			}
			
		}
		
		void testcode ( const Eigen::MatrixXf &s_vertices, std::deque<datapoint> &reconstruction_data ) {
		
			if ( !quit ) {
			
				size_t dims = layers[code_layer_no]->y.rows();
				
				sample_idx.resize ( 1, s_vertices.cols() );
				reconstruction_data.resize ( s_vertices.cols() );
				
				Eigen::VectorXi numbers ( batch_size );
				
				sample_batch.resize ( dims, batch_size );
				
				for ( size_t ii = 0; ii < s_vertices.cols(); ii += batch_size ) {
				
					linspace ( numbers, ii, ii + batch_size );
					make_batch ( sample_batch, s_vertices, numbers );
					
					forward ( code_layer_no, sample_batch );
					
					for ( int b = 0; b < batch_size; b++ )
					
						reconstruction_data[ii + b].x = layers.back()->y.col ( b );
						
						
				}
				
			}
		}
		
		
		NN ( size_t minibatch_size, float _decay = 0.0f, float _learning_rate = 1e-4, network_type type = MLP,
			 std::vector<int> _layer_sizes = {}, nonlinearity_type _ltype = SIGMOID ) :
			batch_size ( minibatch_size ), decay ( _decay ), learning_rate ( _learning_rate ), ntype ( type ) {
			
			layer_sizes = _layer_sizes;
			ltype = _ltype;
			
			otype = SGD;
			
			code_layer_no = 0;
			
			init_net ( ltype );
			
			for ( size_t l = 0; l < layers.size(); l++ ) {
				std::cout << l << ", ";
				layers[l]->layer_info();
			}
			
			
		}
		
		void set_sparsity_penalty ( float p ) {
		
			// for ( size_t l = 0; l < layers.size(); l++ )
			// 	layers[l]->sparsity_penalty = p;
			
		}
		
		void set_sparsity_target ( float t ) {
		
			// for ( size_t l = 0; l < layers.size(); l++ )
			// 	layers[l]->sparsity_target = t;
			
		}
		
		void reset() {
		
			for ( size_t l = 0; l < layers.size(); l++ )
				layers[l]->reset();
		}
		
		void kick() {
		
			for ( size_t l = 0; l < layers.size(); l++ )
				layers[l]->kick();
		}
		
		void init_net ( nonlinearity_type ltype ) {
		
			for ( size_t l = 0; l < layer_sizes.size() - 1; l++ ) {
			
				layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );
				
				if ( ( l + 1 ) == ( layer_sizes.size() - 1 ) ) {
					layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
					// layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
				}
				else {
				
					if ( ltype == SIGMOID )
						layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
					else
						if ( ltype == RELU )
							layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
						else {
						
							std::cout << "init_net(nonlinearity_type ltype): unknown nonlinearity type!" << std::endl;
							layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
						}
				}
				
				
			}
			
		}
		
		void save ( nanogui::Serializer &s ) {
		
			params.lock();
			
			s.set ( "current_loss", current_loss );
			// s.set("loss_data", *loss_data);
			s.set ( "batch_size", batch_size );
			s.set ( "decay", decay );
			s.set ( "ntype", ntype );
			
			s.set ( "batch", batch );
			s.set ( "targets", targets );
			s.set ( "encoding", encoding );
			s.set ( "codes", codes );
			s.set ( "codes_colors", codes_colors );
			s.set ( "codes_idxs", codes_idxs );
			s.set ( "code_layer_no", code_layer_no );
			
			s.set ( "layer_sizes", layer_sizes );
			
			s.set ( "use_code_sigmoid", use_code_sigmoid );
			s.set ( "use_dropout", use_code_sigmoid );
			
			for ( size_t i = 0; i < layers.size(); i++ ) {
			
				s.push ( string_format ( "layer%d", i ) );
				layers[i]->save ( s );
				s.pop();
			}
			
			params.unlock();
			
		}
		
		bool load ( nanogui::Serializer &s ) {
		
			if ( !s.get ( "current_loss", current_loss ) ) return false;
			//if (!s.get("loss_data", *loss_data)) return false;
			if ( !s.get ( "batch_size", batch_size ) ) return false;
			if ( !s.get ( "decay", decay ) ) return false;
			if ( !s.get ( "ntype", ntype ) ) return false;
			
			if ( !s.get ( "batch", batch ) ) return false;
			if ( !s.get ( "targets", targets ) ) return false;
			if ( !s.get ( "encoding", encoding ) ) return false;
			if ( !s.get ( "codes", codes ) ) return false;
			if ( !s.get ( "codes_colors", codes_colors ) ) return false;
			if ( !s.get ( "codes_idxs", codes_idxs ) ) return false;
			if ( !s.get ( "code_layer_no", code_layer_no ) ) return false;
			
			if ( !s.get ( "layer_sizes", layer_sizes ) ) return false;
			
			// if ( !s.get ( "use_code_sigmoid", use_code_sigmoid ) ) return false;
			// if ( !s.get ( "use_dropout", use_dropout ) ) return false;
			
			layers.clear();
			
			//TODO: switch to something supporting polymorphic types
			init_net ( ltype );
			
			for ( size_t i = 0; i < layers.size(); i++ ) {
				s.push ( string_format ( "layer%d", i ) );
				layers[i]->load ( s );
				s.pop();
			}
			
			return true;
		}
		
		
		~NN() {
		
			for ( size_t i = 0; i < layers.size(); i++ )
				delete ( layers[i] );
				
		}
};

#endif
