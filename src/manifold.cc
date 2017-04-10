/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-04-09 21:18:15
*/

#include <thread>
#include <unistd.h>
#include <ctime>

//nanogui
#include <nanogui/screen.h>
#include <serializer.h>

//for NN
#include <io/import.h>
#include <nn/nn.h>

std::shared_ptr<NN> nn;

std::deque<datapoint> train_data;
std::deque<datapoint> reconstruction_data;
std::deque<datapoint> sample_reconstruction_data;
std::deque<datapoint> test_data;

//GUI
#include "gui/manifoldscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	
	PlotData *gl_data = screen->plot_data;
	
	// NN stuff
	double learning_rate = 1e-3;
	float decay = 0;
	const size_t batch_size = 25;
	const int input_width = static_cast<int> ( train_data[0].x.size() );
	assert ( input_width > 0 );
	
	size_t e = 0;
	
	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, { input_width, 256, 64, 32, 16, 3, 16, 32, 64, 256, input_width } ) );
	
	nn->otype = SGD;
	nn->pause = true;
	
	size_t generate_point_count = 10000;
	generate ( std::uniform_real_distribution<> ( 0, 1 ),
			   std::uniform_real_distribution<> ( 0, 1 ),
			   std::uniform_real_distribution<> ( 0, 1 ),
			   gl_data->s_vertices, generate_point_count, INDEPENDENT );
			   
	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->s_colors, generate_point_count );
	
	//bind graph data
	if ( screen ) {
		if ( screen->graph_loss )
			nn->loss_data = screen->graph_loss->values_ptr();
			
	}
	
	// size_t iters = train_data.size() / batch_size;
	size_t iters = 100000;
	
	/* work until main window is open */
	while ( screen->getVisible() ) {
	
		// drawing
		nn->testcode ( train_data, reconstruction_data );
		nn->testcode ( gl_data->s_vertices, sample_reconstruction_data );
		
		gl_data->p_vertices = nn->codes;
		
		// convert labels to colors, TODO: move somewhere else
		gl_data->p_colors.resize ( 3, nn->codes_colors.cols() );
		for ( int k = 0; k < nn->codes_colors.cols(); k++ ) {
			nanogui::Color c = nanogui::parula_lut[ ( int ) nn->codes_colors ( 0, k )];
			gl_data->p_colors.col ( k ) = Eigen::Vector3f ( c[0], c[1], c[2] );
		}
		
		gl_data->updated();
		nn->train ( train_data, iters );
		gl_data->updated();
		
		std::cout << return_current_time_and_date() << std::endl;
		std::string fprefix = "snapshot_last";
		printf ( "Epoch %3lu: Loss: %.2f\n", ++e, ( float ) nn->test ( test_data ) );
		nanogui::Serializer s ( std::string ( "./saved/" + fprefix + ".nn.bin" ).c_str(), true );
		nn->save ( s );
		
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
		
	}
	catch ( const std::runtime_error &e ) {
	
		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;
		
	}
	
	return 0;
}
