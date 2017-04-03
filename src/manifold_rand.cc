/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-31 22:05:58
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>
#include <serializer.h>

//for NN
#include <io/import.h>
#include <nn/nn.h>

std::shared_ptr<NN> nn;

std::deque<datapoint> train_data;
std::deque<datapoint> test_data;

//GUI
#include "gui/manifoldscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

int compute() {

	size_t point_count = 50000;

	PlotData *gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-4;
	float decay = 1e-7;
	const size_t image_size = 28;
	const size_t batch_size = 50;
	size_t e = 0;

	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, {image_size * image_size, 64, 3, 64, image_size * image_size} ) );

	nn->otype = SGD;
	nn->pause = true;

	generate ( std::normal_distribution<> ( 0, 0.35 ),
	           std::normal_distribution<> ( 0, 0.35 ),
	           std::normal_distribution<> ( 0, 0.35 ),
	           gl_data->p_vertices, point_count, STRATIFIED );

	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->p_colors, point_count );

	gl_data->updated();

	/* work until main window is open */
	while ( screen->getVisible() ) {

		// gl_data->updated();
		usleep ( 1000 );

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


	}

	catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}