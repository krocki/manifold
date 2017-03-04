/*
* @Author: kmrocki
* @Date:   2016-02-24 15:28:10
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 21:28:13
*/

#ifndef __NN_H__
#define __NN_H__

#include <nn/layers.h>

/* temporary */
extern Eigen::MatrixXf image_data[4];
extern bool quit;
/* temporary */

class NN {

  public:

	std::deque<Layer*> layers;

	const size_t batch_size;

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

		for (size_t ii = 0; ii < iterations; ii++) {

			randi(random_numbers, 0, data.size() - 1);

			// [784 x batch_size]
			Matrix batch = make_batch(data, random_numbers);
			Matrix targets = make_targets(data, random_numbers, classes);

			/* temporary */

			// for (size_t i = 0; i < 4; i++) {
			// 	Eigen::Map<Eigen::MatrixXf, Eigen::ColMajor> A(batch.col(i).data(), 28, 28);
			// 	image_data[i] = A;
			// }
			/* temporary */

			//forward activations
			forward(batch);

			double loss = cross_entropy(layers[layers.size() - 1]->y, targets);
			std::cout << "[" << ii + 1 << "/" << iterations << "] Loss = " << loss << std::endl;

			//backprogagation
			backward(targets);

			//apply changes
			update(alpha);

			if (quit) break;

		}

	}


	void test(std::deque<datapoint> data) {

		Eigen::VectorXi numbers(batch_size);
		size_t classes = 10;
		size_t correct = 0;

		for (size_t ii = 0; ii < data.size(); ii += batch_size) {

			linspace(numbers, ii, ii + batch_size);

			Matrix batch = make_batch(data, numbers);
			Matrix targets = make_targets(data, numbers, classes);

			forward(batch);

			correct += count_correct_predictions(layers[layers.size() - 1]->y, targets);


		}

		std::cout << "Test % correct = " << 100.0 * (double)correct / (double)(data.size()) << std::endl;

	}

	NN(size_t minibatch_size) : batch_size(minibatch_size) { }

	~NN() {

		for (size_t i = 0; i < layers.size(); i++) {

			delete(layers[i]);
		}

	}
};

#endif