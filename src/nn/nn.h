/*
* @Author: kmrocki
* @Date:   2016-02-24 15:28:10
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-07 20:35:18
*/

#ifndef __NN_H__
#define __NN_H__

#include <nn/layers.h>

#include "perf.h"

class NN {

public:

	std::deque<Layer*> layers;
	float current_loss = -0.01f;

	bool clock = false;
	bool quit = false;
	bool pause = false;
	bool step = false;

	const size_t batch_size;

	Matrix batch;
	Matrix targets;
	Matrix encoding;

	void forward(Matrix& input_data) {

		//copy inputs to the lowest point in the network
		layers[0]->x = input_data;

		//compute forward activations
		for (size_t i = 0; i < layers.size(); i++) {

			//y = f(x)
			layers[i]->forward();

			//x(next layer) = y(current layer)
			if (i + 1 < layers.size())
				layers[i + 1]->x = layers[i]->y;
		}

	}

	void backward(Matrix t) {

		//set targets at the top
		layers[layers.size() - 1]->dy = t;

		//propagate error backward
		for (int i = layers.size() - 1; i >= 0; i--) {

			layers[i]->resetGrads();
			layers[i]->backward();

			//dy(previous layer) = dx(current layer)
			if (i > 0) {
				layers[i - 1]->dy = layers[i]->dx;
			}

		}

	}

	void update(double alpha) {

		//update all layers according to gradients
		for (size_t i = 0; i < layers.size(); i++) {

			layers[i]->applyGrads(alpha);

		}

	}

	void train(std::deque<datapoint> data, double alpha, size_t iterations) {

		//get random examples of size batch_size from data
		Eigen::VectorXi random_numbers(batch_size);
		size_t classes = 10;

		batch.resize(data[0].x.rows(), batch_size);
		targets.resize(classes, batch_size);
		encoding = Matrix::Identity(classes, classes);

		for (size_t ii = 0; ii < iterations; ii++) {

			tic();

			randi(random_numbers, 0, data.size() - 1);

			// [784 x batch_size]
			make_batch(batch, data, random_numbers);
			make_targets(targets, encoding, data, random_numbers);

			ticf();

			//forward activations
			forward(batch);

			double ce = cross_entropy(layers[layers.size() - 1]->y, targets);
			current_loss = current_loss < 0 ? ce : 0.99 * current_loss + 0.01 * ce;

			// std::cout << "[" << ii + 1 << "/" << iterations << "] Loss = " << current_loss << std::endl;

			if (ii % 5 == 0) clock = true;

			//backprogagation
			backward(targets);

			//apply changes
			update(alpha);

			tocf();
			toc();

			if (quit) break;

			while (pause) {
				usleep(10000);
				if (step || quit) { step = false; break; }
			}
		}

	}


	double test(std::deque<datapoint> data) {

		Eigen::VectorXi numbers(batch_size);
		size_t classes = 10;
		size_t correct = 0;

		batch.resize(data[0].x.rows(), batch_size);
		targets.resize(classes, batch_size);
		encoding = Matrix::Identity(classes, classes);

		for (size_t ii = 0; ii < data.size(); ii += batch_size) {

			linspace(numbers, ii, ii + batch_size);

			make_batch(batch, data, numbers);
			make_targets(targets, encoding, data, numbers);

			forward(batch);

			correct += count_correct_predictions(layers[layers.size() - 1]->y, targets);


		}

		std::cout << "Test % correct = " << 100.0 * (double)correct / (double)(data.size()) << std::endl;

		return (double)correct / (double)(data.size());
	}

	NN(size_t minibatch_size) : batch_size(minibatch_size) { }

	~NN() {

		for (size_t i = 0; i < layers.size(); i++) {

			delete(layers[i]);
		}

	}
};

#endif