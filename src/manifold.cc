/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-24 14:00:00
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/manifoldscreen.h"
#include <compute/functions.h>

GUI *screen;

//for NN
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

NN *nn;

int compute() {

	PlotData* gl_data = screen->plot_data;
	gl_data->updated();

	// NN stuff
	double learning_rate = 1e-4;
	float decay = 0;
	const size_t image_size = 28;
	const size_t batch_size = 100;
	size_t l = 0;

	std::vector<int> layer_sizes = {image_size * image_size, 100, 3, 2, 3, 100,  image_size * image_size};
	nn = new NN ( batch_size, decay, AE );

	nn->code_layer_no = 2 * ( layer_sizes.size() - 1 ) / 2 - 1;
	std::cout << "CODE LAYER: " << nn->code_layer_no << std::endl;

	for ( l = 0; l < layer_sizes.size() - 1; l++ ) {

		nn->layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );

		if ( ( l + 1 ) == layer_sizes.size() - 1 )
			nn->layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
		else
			nn->layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
	}

	// DATA
	nn->train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
	nn->test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

	nn->testcode ( nn->train_data );

	std::cout << "nn ready" << std::endl;
	nn->setready();

	/* work until main window is open */
	while (screen->getVisible()) {

		usleep(1000);
		//gl_data->updated();

	}

	delete nn; return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		screen = new GUI();

		// launch a compute thread
		std::thread compute_thread(compute);

		nanogui::mainloop ( 1 );

		delete screen;
		nanogui::shutdown();
		compute_thread.join();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}