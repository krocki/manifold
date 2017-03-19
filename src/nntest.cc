/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:06:14
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-09 12:55:59
*/

#include <iostream>

#include <utils.h>
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

int main() {

	size_t epochs = 100;
	size_t batch_size = 250;
	double learning_rate = 1e-3;
	
	NN nn ( batch_size );
	
	nn.layers.push_back ( new Linear ( 28 * 28, 256, batch_size ) );
	nn.layers.push_back ( new ReLU ( 256, 256, batch_size ) );
	nn.layers.push_back ( new Linear ( 256, 256, batch_size ) );
	nn.layers.push_back ( new ReLU ( 256, 256, batch_size ) );
	nn.layers.push_back ( new Linear ( 256, 100, batch_size ) );
	nn.layers.push_back ( new ReLU ( 100, 100, batch_size ) );
	nn.layers.push_back ( new Linear ( 100, 10, batch_size ) );
	nn.layers.push_back ( new Softmax ( 10, 10, batch_size ) );
	
	//[60000, 784]
	std::deque<datapoint> train_data =
		MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte",
										"data/mnist/train-labels-idx1-ubyte" );
	//[10000, 784]
	std::deque<datapoint> test_data =
		MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte",
										"data/mnist/t10k-labels-idx1-ubyte" );
										
	for ( size_t e = 0; e < epochs; e++ ) {
	
		nn.train ( train_data, learning_rate, train_data.size() / batch_size );
		float acc = nn.test ( test_data );
		printf ( "Epoch %3lu: %.2f %%\n", e + 1, 100.0f * acc );
		
	}
	
}