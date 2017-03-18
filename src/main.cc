/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-17 18:19:36
*/

#include <thread>
#include <unistd.h>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>

//helpers
#include <utils.h>

//for NN
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

NN *nn;

int screen_scale = 1;
const size_t batch_size = 100;
const size_t image_size = 28;

nanogui::Screen *screen;

#include "gui/plotpoints_alt.h"
#include "gui/manifoldscreen.h"

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization
	
	double learning_rate = 1e-3;
	float decay = 1e-7;
	
	// std::vector<int> layer_sizes = {image_size * image_size, 256, 256, 100, 64, 16, 8, 3, 8, 16, 64, 100, 256, 256, image_size * image_size};
	std::vector<int> layer_sizes = {image_size * image_size, 64, 2, 64, image_size * image_size};
	nn = new NN ( batch_size, decay, AE );
	
	nn->code_layer_no = 2 * ( layer_sizes.size() - 1 ) / 2 - 1;
	std::cout << nn->code_layer_no << std::endl;
	
	size_t l = 0;
	
	for ( l = 0; l < layer_sizes.size() - 1; l++ ) {
	
	
	
		// semantic hashing, gaussian noise
		// if ( ( l + 1 )  == nn->code_layer_no )
		
		// 	nn->layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size, true ) );
		
		// else
		
		nn->layers.push_back ( new Linear ( layer_sizes[l], layer_sizes[l + 1], batch_size ) );
		
		if ( ( l + 1 ) == layer_sizes.size() - 1 )
			nn->layers.push_back ( new Sigmoid ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
		else
			nn->layers.push_back ( new ReLU ( layer_sizes[l + 1], layer_sizes[l + 1], batch_size ) );
	}
	
	//[60000, 784]
	nn->train_data =
		MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
		
	//[10000, 784]
	nn->test_data =
		MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );
		
	nn->testcode ( nn->train_data );
	
	std::cout << "nn ready" << std::endl;
	nn->setready();
	
	for ( size_t e = 0; true; e++ ) {
	
		nn->train ( nn->train_data, learning_rate, nn->train_data.size() / batch_size );
		nn->testcode ( nn->train_data );
		
		if ( nn->quit ) break;
		
		printf ( "Epoch %3lu: Loss: %.2f\n", e + 1, ( float ) nn->test ( nn->test_data ) );
		
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
		
	}
	catch ( const std::runtime_error &e ) {
	
		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;
		
	}
	
	return 0;
	
}