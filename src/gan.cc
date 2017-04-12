/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-12 09:37:29
*/

#include <thread>
#include <unistd.h>
#include <ctime>

//nanogui
#include <nanogui/screen.h>
#include <serializer.h>

//for NN
#include <io/import.h>
#include <nn/gan.h>

std::shared_ptr<NN> nn;
std::shared_ptr<NN> discriminator;

std::deque<datapoint> train_data;
std::deque<datapoint> reconstruction_data;
std::deque<datapoint> sample_reconstruction_data;
std::deque<datapoint> test_data;

//GUI
#include "gui/ganscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	PlotData *gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-3;
	float decay = 0;
	const size_t batch_size = 100;
	const int input_width = static_cast<int> ( train_data[0].x.size() );
	assert ( input_width > 0 );

	size_t e = 0;

	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, {3, 16, 32, input_width } ) );
	discriminator = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, MLP, {input_width, 32, 1 } ) );

	nn->otype = SGD;
	nn->pause = true;
	discriminator->otype = SGD;
	discriminator->pause = true;

	size_t generate_point_count = 10000;
	generate ( std::uniform_real_distribution<> ( 0, 1 ),
	           std::uniform_real_distribution<> ( 0, 1 ),
	           std::uniform_real_distribution<> ( 0, 1 ),
	           gl_data->s_vertices, generate_point_count, INDEPENDENT );

	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->s_colors, generate_point_count );

	if ( screen )
		if ( screen->nnview ) {
			screen->nnview->setnets ( nn, discriminator );
		}

	//bind graph data
	if ( screen ) {
		if ( screen->graph_loss )
			nn->loss_data = screen->graph_loss->values_ptr();

	}

	/* work until main window is open */
	while ( screen->getVisible() ) {

		nn->train ( gl_data->s_vertices, sample_reconstruction_data );

		std::cout << nn->layers.back()->y.rows() << ", " << nn->layers.back()->y.cols() << std::endl;

		gl_data->updated();

		usleep ( 100 );

	}

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		screen = std::shared_ptr<GUI> ( new GUI() );

		// launch a compute thread
		std::thread compute_thread ( compute );

		nanogui::mainloop ( 1 );

		compute_thread.join();
		nanogui::shutdown();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}
