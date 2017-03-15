/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-14 21:50:22
*/

#include <thread>
#include <unistd.h>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>

//helpers
#include <utils.h>

//for NN
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

NN *nn;

int screen_scale = 1;
const size_t batch_size = 50;
const size_t image_size = 28;

nanogui::Screen *screen;

#include "gui/plotpoints_alt.h"
#include "gui/manifoldscreen.h"

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization

	double learning_rate = 1e-3;
	float decay = 0;

	std::vector<int> layer_sizes = {image_size * image_size, 100, 3, 100, image_size * image_size};
	nn = new NN ( batch_size, decay, DAE );

	nn->code_layer_no = 3;


	nn->code_layer_no = 3;

	size_t l = 0;

	for ( l = 0; l < layer_sizes.size() - 1; l++ ) {

		nn->layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );

		if ( ( l + 1 ) == layer_sizes.size() - 1 )
			nn->layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
		else
			nn->layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
	}

	//[60000, 784]
	std::deque<datapoint> train_data =
	    MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );

	//[10000, 784]
	std::deque<datapoint> test_data =
	    MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

	nn->testcode ( train_data );

	std::cout << "nn ready" << std::endl;
	nn->setready();

	for ( size_t e = 0; true; e++ ) {

		nn->train ( train_data, learning_rate, train_data.size() / batch_size );
		nn->testcode ( train_data );

		if ( nn->quit ) break;

		printf ( "Epoch %3lu: Loss: %.2f\n", e + 1, ( float ) nn->test ( test_data ) );

	}

	nn->quit = true;

	//should wait for GL to finish first before deleting nn
	delete nn; return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();

		// launch a compute thread
		std::thread compute_thread ( compute );

		// wait until nn is allocated
		while ( ! nn ) {
			std::cout << "main" << std::endl;
			usleep ( 1000 );
		}

		screen = new GUI();

		nanogui::mainloop ( 1 );

		delete screen;
		nanogui::shutdown();
		compute_thread.join();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;

}