/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-24 19:24:42
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
#include <io/import.h>
#include <nn/nn.h>

NN *nn;

int compute() {

	PlotData* gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-3;
	float decay = 0;
	const size_t image_size = 28;
	const size_t batch_size = 100;
	size_t e = 0;

	// DATA
	std::deque<datapoint> train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
	std::deque<datapoint> test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

	nn = new NN ( batch_size, decay, AE, {image_size * image_size, 64, 3, 64, image_size * image_size});

	nn->testcode ( train_data );
	nn->setready();
	gl_data->updated();

	//bind graph data
	nn->loss_data = screen->plot_helper->graph_loss->values_ptr();

	/* work until main window is open */
	while (screen->getVisible()) {

		nn->train ( train_data, learning_rate, train_data.size() / batch_size );
		nn->testcode ( train_data );
		gl_data->p_vertices = nn->codes;

		// convert labels to colors, TODO: move somewhere else
		gl_data->p_colors.resize(3, nn->codes_colors.cols());
		for (int k = 0; k < nn->codes_colors.cols(); k++) {
			nanogui::Color c = nanogui::parula_lut[(int)nn->codes_colors(0, k)];
			gl_data->p_colors.col(k) = Eigen::Vector3f(c[0], c[1], c[2]);
		}

		gl_data->updated();

		printf ( "Epoch %3lu: Loss: %.2f\n", ++e, ( float ) nn->test ( test_data ) );
		usleep(1000);

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