/*
* @Author: kmrocki
* @Date:   2016-02-24 15:28:10
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-26 13:37:36
*/

#ifndef __NN_H__
#define __NN_H__

#include <io/import.h>
#include <nn/layers.h>
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

	bool clock = false;
	bool quit = false;
	bool pause = true;
	bool step = false;

	int batch_size;
	float decay;
	network_type ntype;

	Matrix batch;
	Matrix targets;
	Matrix encoding;
	Matrix codes, codes_colors;
	Eigen::MatrixXi codes_idxs;

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

	void backward ( const Matrix& t ) {

		//set targets at the top
		layers[layers.size() - 1]->dy = t;

		//propagate error backward
		for ( int i = layers.size() - 1; i >= 0; i-- ) {

			layers[i]->resetGrads();
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

	void train ( const std::deque<datapoint>& data, double alpha, size_t iterations ) {

		size_t classes = 10;
		Eigen::VectorXi random_numbers ( batch_size );
		batch.resize ( data[0].x.rows(), batch_size );
		// size_t dims = layers[code_layer_no]->y.rows();

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
				if (loss_data) {

					loss_data->head ( loss_data->size() - 1 ) = loss_data->tail ( loss_data->size() - 1 );
					loss_data->tail ( 1 ) ( 0 ) = current_loss;

				}
			}

			//apply changes
			update ( alpha, decay );

			tocf();
			toc();

			if ( quit ) break;

			while ( pause ) {
				usleep ( 10000 );
				if ( step || quit ) { step = false; break; }
			}

		}
	}


	double test ( const std::deque<datapoint>& data ) {

		while ( pause ) {
			usleep ( 10000 );
			if ( step || quit ) { step = false; break; }
		}

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

	void testcode ( const std::deque<datapoint>& data ) {

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

	NN ( size_t minibatch_size, float decay = 0.0f, network_type type = MLP, std::vector<int> _layer_sizes = {}) : batch_size ( minibatch_size ), decay ( decay ), ntype ( type ) {

		layer_sizes = _layer_sizes;

		code_layer_no = 2 * ( layer_sizes.size() - 1 ) / 2 - 1;

		for ( size_t l = 0; l < layer_sizes.size() - 1; l++ ) {

			layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );

			if ( ( l + 1 ) == layer_sizes.size() - 1 )
				layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
			else
				layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );

		}

	}

	void save(nanogui::Serializer &s) {

		s.set("current_loss", current_loss);
		// s.set("loss_data", *loss_data);
		s.set("batch_size", batch_size);
		s.set("decay", decay);
		s.set("ntype", ntype);

		s.set("batch", batch);
		s.set("targets", targets);
		s.set("encoding", encoding);
		s.set("codes", codes);
		s.set("codes_colors", codes_colors);
		s.set("codes_idxs", codes_idxs);
		s.set("code_layer_no", code_layer_no);

		s.set("layer_sizes", layer_sizes);

		params.lock();

		for ( size_t i = 0; i < layers.size(); i++ ) {

			s.push(string_format ( "layer%d", i));
			layers[i]->save(s);
			s.pop();
		}

		params.unlock();

	}

	bool load(nanogui::Serializer &s) {

		if (!s.get("current_loss", current_loss)) return false;
		//if (!s.get("loss_data", *loss_data)) return false;
		if (!s.get("batch_size", batch_size)) return false;
		if (!s.get("decay", decay)) return false;
		if (!s.get("ntype", ntype)) return false;

		if (!s.get("batch", batch)) return false;
		if (!s.get("targets", targets)) return false;
		if (!s.get("encoding", encoding)) return false;
		if (!s.get("codes", codes)) return false;
		if (!s.get("codes_colors", codes_colors)) return false;
		if (!s.get("codes_idxs", codes_idxs)) return false;
		if (!s.get("code_layer_no", code_layer_no)) return false;

		if (!s.get("layer_sizes", layer_sizes)) return false;

		layers.clear();

		//TODO: switch to something supporting polymorphic types
		for ( size_t l = 0; l < layer_sizes.size() - 1; l++ ) {

			layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );

			if ( ( l + 1 ) == layer_sizes.size() - 1 )
				layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
			else
				layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );

		}

		for ( size_t i = 0; i < layers.size(); i++ ) {
			s.push(string_format ( "layer%d", i));
			layers[i]->load(s);
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
