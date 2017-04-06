/*
* @Author: kmrocki
* @Date:   2016-02-24 15:28:10
* @Last Modified by:   kamilrocki
* @Last Modified time: 2017-04-05 23:09:09
*/

#ifndef __NN_H__
#define __NN_H__

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

class NN {

  public:

	std::vector<int> layer_sizes;
	std::deque<Layer *> layers;
	float current_loss = -0.01f;
	Eigen::VectorXf *loss_data;

	float learning_rate;

	bool use_code_sigmoid = false;
	bool use_dropout = false;

	bool clock = false;
	bool quit = false;
	bool pause = false;
	bool step = false;

	int batch_size;
	float decay;

	network_type ntype;
	opt_type otype;

	Eigen::VectorXi random_numbers;

	Matrix batch;
	Matrix targets;
	Matrix encoding;
	Matrix codes, codes_colors;
	Eigen::MatrixXi codes_idxs;

	float epoch_progress;

	int code_layer_no = 1;

	std::mutex params;

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
			// std::cout << "layer " << i << std::endl;
			// layers[i]->sparsify();
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

	void train ( const std::deque<datapoint> &data, size_t iterations ) {

		size_t classes = 10;
		random_numbers.resize ( batch_size );
		batch.resize ( data[0].x.rows(), batch_size );
		// size_t dims = layers[code_layer_no]->y.rows();

		if ( !quit ) {
			if ( ntype == AE || ntype == DAE ) {

				// codes.resize ( dims, iterations * batch_size );
				// codes_colors.resize ( 1, iterations * batch_size );

				// targets.resize ( 1, batch_size );
				// encoding.resize ( 1, 10 );
				// encoding << 0, 1, 2, 3, 4, 5, 6, 7, 8, 9;

			} else {

				targets.resize ( classes, batch_size );
				encoding = Matrix::Identity ( classes, classes );

			}

			for ( size_t ii = 0; ii < iterations; ii++ ) {

				tic();

				matrix_randi ( random_numbers, 0, data.size() - 1 );

				// [784 x batch_size]
				make_batch ( batch, data, random_numbers );

				if ( ntype == MLP ) make_targets ( targets, encoding, data, random_numbers );

				ticf();

				//forward activations
				if ( ntype == DAE ) {

					/* Denoising */
					Matrix mask ( batch.rows(), batch.cols() );
					random_binary_mask ( mask );
					Matrix corrupted_batch ( batch.rows(), batch.cols() );
					corrupted_batch.array() = batch.array() * mask.array();
					forward ( corrupted_batch );

				} else

					forward ( batch );

				double err;

				//backprogagation
				if ( ntype == AE || ntype == DAE ) {

					// codes.block ( 0, ii * batch_size, dims, batch_size ) = layers[code_layer_no]->y;
					// codes_colors.block ( 0, ii * batch_size, 1, batch_size ) = targets;

					err = mse ( layers[layers.size() - 1]->y, batch ) / ( float ) batch_size;

					// reconstruct
					backward ( batch - layers[layers.size() - 1]->y );


				} else {

					err = cross_entropy ( layers[layers.size() - 1]->y, targets );

					backward ( targets );

				}

				current_loss = current_loss < 0 ? err : 0.99 * current_loss + 0.01 * err;

				if ( ii % 50 == 0 ) {

					clock = true;
					//update graph data
					if ( loss_data ) {

						loss_data->head ( loss_data->size() - 1 ) = loss_data->tail ( loss_data->size() - 1 );
						loss_data->tail ( 1 ) ( 0 ) = current_loss;

					}

					// perturb net randomly
					// kick();

				}

				//apply changes
				update ( learning_rate, decay );



				tocf();
				toc();

				while ( pause ) {
					usleep ( 10000 );
					if ( step || quit ) { step = false; break; }
				}

				epoch_progress = ((float)ii) / (float)iterations;
				printf("\t%.1f%%\r", epoch_progress * 100.0);
				fflush(stdout);

			}
		}
	}


	double test ( const std::deque<datapoint> &data ) {

		while ( pause ) {
			usleep ( 10000 );
			if ( step || quit ) { step = false; break; }
		}

		if ( !quit ) {

			if ( ntype == AE || ntype == DAE )

				return current_loss;

			else {

				Eigen::VectorXi numbers ( batch_size );
				size_t classes = 10;
				size_t correct = 0;

				batch.resize ( data[0].x.rows(), batch_size );
				targets.resize ( classes, batch_size );
				encoding = Matrix::Identity ( classes, classes );

				for ( size_t ii = 0; ii < data.size(); ii += batch_size ) {

					linspace ( numbers, ii, ii + batch_size );

					make_batch ( batch, data, numbers );
					make_targets ( targets, encoding, data, numbers );

					forward ( batch );

					correct += count_correct_predictions ( layers[layers.size() - 1]->y, targets );


				}

				return ( double ) correct / ( double ) ( data.size() );
			}
		}

		return 0.0;
	}

	void testcode ( const std::deque<datapoint> &data ) {

		if ( !quit ) {

			size_t dims = layers[code_layer_no]->y.rows();

			codes.resize ( dims, data.size() );
			codes_colors.resize ( 1, data.size() );
			codes_idxs.resize ( 1, data.size() );

			Eigen::VectorXi numbers ( batch_size );

			batch.resize ( data[0].x.rows(), batch_size );
			targets.resize ( 1, batch_size );
			encoding.resize ( 1, 10 );
			encoding << 0, 1, 2, 3, 4, 5, 6, 7, 8, 9;

			for ( size_t ii = 0; ii < data.size(); ii += batch_size ) {

				linspace ( numbers, ii, ii + batch_size );
				make_batch ( batch, data, numbers );

				if ( ntype == DAE ) batch /= 2.0f;

				make_targets ( targets, encoding, data, numbers );

				forward ( batch, code_layer_no );

				codes.block ( 0, ii, dims, batch_size ) = layers[code_layer_no]->y;
				codes_colors.block ( 0, ii, 1, batch_size ) = targets;
				codes_idxs.block ( 0, ii, 1, batch_size ) = numbers.transpose();

			}

		}
	}

	NN ( size_t minibatch_size, float _decay = 0.0f, float _learning_rate = 1e-4, network_type type = MLP, std::vector<int> _layer_sizes = {} ) :
		batch_size ( minibatch_size ), decay ( _decay ), learning_rate ( _learning_rate ), ntype ( type ) {

		layer_sizes = _layer_sizes;

		otype = SGD;

		//dropout
		// if (use_dropout)
		// 	code_layer_no = 3 * ( layer_sizes.size() - 1 ) / 2 - 2;
		// else
		code_layer_no = 2 * ( layer_sizes.size() - 1 ) / 2 - 1;

		std::cout << code_layer_no << std::endl;

		init_net();

		for ( size_t l = 0; l < layers.size(); l++ ) {
			std::cout << l << ", ";
			layers[l]->layer_info();
		}


	}

	void set_sparsity_penalty(float p) {

		// for ( size_t l = 0; l < layers.size(); l++ )
		// 	layers[l]->sparsity_penalty = p;

	}

	void set_sparsity_target(float t) {

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

	void init_net() {

		for ( size_t l = 0; l < layer_sizes.size() - 1; l++ ) {

			layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size) );
			// layers.back()->adjust_sparsity = false;

			if ( ( l + 1 ) == ( layer_sizes.size() - 1 ) )
				layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
			else {

				// if (use_code_sigmoid &&  int ( layers.size() ) == code_layer_no )
				// 	layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
				// else {
				layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );

				// }

				//dropout
				// if (use_dropout) {
				// 	if ( int ( layers.size() - 1 ) != code_layer_no )
				// 		layers.push_back ( new Dropout ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size, 1.0f ) );
				// }
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
		init_net();

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
