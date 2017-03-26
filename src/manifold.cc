/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-26 10:12:09
*/

#include <thread>
#include <unistd.h>
#include <ctime>

//nanogui
#include <nanogui/screen.h>
#include <serializer.h>

//GUI
#include "gui/manifoldscreen.h"
#include <compute/functions.h>

GUI *screen;

//for NN
#include <io/import.h>
#include <nn/nn.h>

NN *nn;

int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	PlotData* gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-5;
	float decay = 0;
	const size_t image_size = 28;
	const size_t batch_size = 100;
	size_t e = 0;

	// DATA
	std::deque<datapoint> train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
	std::deque<datapoint> test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

	nn = new NN ( batch_size, decay, DAE, {image_size * image_size, 256, 128, 64, 3, 64, 128, 256, image_size * image_size});

	//bind graph data
	//nn->loss_data = screen->plot_helper->graph_loss->values_ptr();

	// nanogui::Serializer s_read(string_format ( "snapshots/170325_132952_61.bin" ), false);
	// nn->load(s_read);

	size_t iters = train_data.size() / batch_size;

	/* work until main window is open */
	while (screen->getVisible()) {

		// drawing
		nn->testcode ( train_data );
		gl_data->updated();
		gl_data->p_vertices = nn->codes;

		// convert labels to colors, TODO: move somewhere else
		gl_data->p_colors.resize(3, nn->codes_colors.cols());
		for (int k = 0; k < nn->codes_colors.cols(); k++) {
			nanogui::Color c = nanogui::parula_lut[(int)nn->codes_colors(0, k)];
			gl_data->p_colors.col(k) = Eigen::Vector3f(c[0], c[1], c[2]);
		}

		nn->train ( train_data, learning_rate, iters );

		std::cout << return_current_time_and_date() << std::endl;
		printf ( "Epoch %3lu: Loss: %.2f\n", ++e, ( float ) nn->test ( test_data ) );

		usleep(1000);

	}

	// save last state
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
	std::cout << "End time: " << t << ", " << "elapsed time: " << elapsed_seconds.count() << std::endl;
	std::string fprefix = string_format ( "snapshots/%s_%d_%d", t.c_str(), (int)elapsed_seconds.count(), (int)nn->current_loss);
	nanogui::Serializer s(string_format ( "%s.nn.bin", fprefix.c_str()), true);
	nn->save(s);
	std::cout << "Done " << std::endl;

	std::fstream fin("dump.png", ios::in | ios::binary);
	std::fstream fout(string_format ( "%s.png", fprefix.c_str()), ios::out | ios::binary);

	char c;
	while (!fin.eof()) {
		fin.get(c);
		fout.put(c);
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

		compute_thread.join();
		delete screen;
		nanogui::shutdown();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}
