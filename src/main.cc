/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-04 13:54:24
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <thread>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/theme.h>
#include <nanogui/graph.h>
#include <nanogui/matrix.h>
#include <nanogui/console.h>

//for NN
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

//performance counters
#include "fps.h"
#include "perf.h"

// rand
#include "aux.h"

//helpers
#include <utils.h>

#define SCREEN_DEFAULT_WIDTH 640
#define SCREEN_DEFAULT_HEIGHT 480
#define SCREEN_NAME "VAE"

bool quit = false;

Eigen::MatrixXf image_data[4];

class Manifold : public nanogui::Screen {

  public:

	Manifold ( bool fullscreen = false, int aliasing_samples = 8 ) :

		nanogui::Screen ( Eigen::Vector2i ( SCREEN_DEFAULT_WIDTH, SCREEN_DEFAULT_HEIGHT ),
		                  SCREEN_NAME, true, fullscreen, 8, 8, 24, 8, aliasing_samples, 3, 3 ) {
		init();

	}

	void console ( const char *pMsg, ... ) {

		char buffer[4096];
		std::va_list arg;
		va_start ( arg, pMsg );
		std::vsnprintf ( buffer, 4096, pMsg, arg );
		va_end ( arg );
		log_str.append ( buffer );

	}

	void init() {

		int glfw_window_width, glfw_window_height;

		glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

		// get GL debug info
		console ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		console ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		console ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		console ( "glfwGetWindowSize(): %d x %d\n", glfw_window_width, glfw_window_height );

		//FPS graph
		graph_fps = add<nanogui::Graph> ( "" );
		graph_fps->values().resize ( FPS.size() );
		graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

		//CPU graph
		graph_cpu = add<nanogui::Graph> ( "" );
		graph_cpu->values().resize ( cpu_util.size() );
		graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

		// FlOP/s
		graph_flops = add<nanogui::Graph> ( "" );
		graph_flops->values().resize ( cpu_flops.size() );
		graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

		/* console */
		show_console = false;

		console_window = new nanogui::Window ( this, "" );
		console_window->setVisible ( show_console );

		// console_window->setLayout ( new GroupLayout ( 5, 5, 0, 0 ) );
		console_panel = new nanogui::Console ( console_window );
		console_panel->setFontSize ( 12 );

		// debug footer message
		footer_message = new nanogui::Console ( this );
		footer_message->setFontSize ( 12 );
		footer_message_string = "";

		/* temporary */
		// for (size_t i = 0; i < 4; i++) {

		// 	image_data[i].resize(28, 28);
		// 	mplot[i] = add<nanogui::MatrixPlot> ( "image" );
		// 	mplot[i]->setPosition ( {50 + i * 100, 50} );
		// 	mplot[i]->setSize ( {95, 95} );
		// 	mplot[i]->setShadeColor ( nanogui::Color ( 64, 192, 160, 255 ) );
		// 	mplot[i]->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 0 ) );
		// 	mplot[i]->values().resize ( image_data[i].rows(), image_data[i].cols() );
		// }
		/* temporary */

		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}

	~Manifold() { }

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {
		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			setVisible ( false );
			return true;
		}

		if ( key == GLFW_KEY_TAB && action == GLFW_PRESS ) {

			show_console = !show_console;
			console_window->setVisible ( show_console );

		}

		return false;
	}

	virtual void draw ( NVGcontext *ctx ) {

		graph_fps->values() = Eigen::Map<Eigen::VectorXf> ( FPS.data(), FPS.size() );
		graph_cpu->values() = Eigen::Map<Eigen::VectorXf> ( cpu_util.data(), cpu_util.size() );
		graph_flops->values() = Eigen::Map<Eigen::VectorXf> ( cpu_flops.data(), cpu_flops.size() );

		char str[16];
		int last_avg = 10;

		sprintf ( str, "%3.1f FPS\n", graph_fps->values().block ( FPS.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );
		graph_fps->setHeader ( str );

		sprintf ( str, "%5.1f ms\n", 1000.0 * graph_cpu->values().block ( cpu_util.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );
		graph_cpu->setHeader ( str );

		sprintf ( str, "%5.1f GF/s\n", graph_flops->values().block ( cpu_flops.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );
		graph_flops->setHeader ( str );

		console_panel->setValue ( log_str );

		/* temporary */
		// for (size_t i = 0; i < 4; i++)
		// 	mplot[i]->values() = image_data[i];

		// debug
		std::stringstream ss;
		ss << size() [0] << "x" << size() [1] << "\n";

		footer_message_string = ss.str();
		footer_message->setValue ( footer_message_string );

		/* Draw the user interface */
		Screen::draw ( ctx );
		update_FPS();

	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		int graph_width = 110;
		int graph_height = 15;

		// FPS graph
		graph_fps->setPosition ( {5, size[1] - graph_height - 5} );
		graph_fps->setSize ( {graph_width, graph_height } );

		// CPU graph
		graph_cpu->setPosition ( {5 + graph_width + 5, size[1] - graph_height - 5} );
		graph_cpu->setSize ( {graph_width, graph_height } );

		// GFLOPS graph
		graph_flops->setPosition ( {5 + (graph_width + 5) * 2, size[1] - graph_height - 5} );
		graph_flops->setSize ( {graph_width, graph_height } );

		// console
		int console_width = 250;
		int console_height = size[1] - 10;
		console_window->setPosition ( {size[0] - console_width - 5, 5} );
		console_window->setSize ( {console_width, console_height} );
		console_panel->setPosition ( {5, 5} );
		console_panel->setWidth ( console_width - 10 );
		console_panel->setHeight ( console_height - 10 );

		// footer
		int footer_height = 40;
		int footer_width = 550;
		footer_message->setPosition ( {5, 5} );
		footer_message->setSize ( {footer_width, footer_height} );

		performLayout();

		return true;

	}

	virtual void drawContents() { }

	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops;
	nanogui::Window *console_window;
	nanogui::Console *console_panel, *footer_message;
	nanogui::MatrixPlot *mplot[4];

	std::string log_str, footer_message_string;

	bool show_console;
};

int compute() {

	size_t epochs = 100;
	size_t batch_size = 250;
	double learning_rate = 1e-3;

	NN nn(batch_size);

	nn.layers.push_back(new Linear(28 * 28, 256, batch_size));
	nn.layers.push_back(new ReLU(256, 256, batch_size));
	nn.layers.push_back(new Linear(256, 256, batch_size));
	nn.layers.push_back(new ReLU(256, 256, batch_size));
	nn.layers.push_back(new Linear(256, 100, batch_size));
	nn.layers.push_back(new ReLU(100, 100, batch_size));
	nn.layers.push_back(new Linear(100, 10, batch_size));
	nn.layers.push_back(new Softmax(10, 10, batch_size));

	//[60000, 784]
	std::deque<datapoint> train_data =
	    MNISTImporter::importFromFile("data/mnist/train-images-idx3-ubyte",
	                                  "data/mnist/train-labels-idx1-ubyte");
	//[10000, 784]
	std::deque<datapoint> test_data =
	    MNISTImporter::importFromFile("data/mnist/t10k-images-idx3-ubyte",
	                                  "data/mnist/t10k-labels-idx1-ubyte");

	for (size_t e = 0; e < epochs; e++) {

		std::cout << "Epoch " << e + 1 << std::endl << std::endl;
		nn.train(train_data, learning_rate, train_data.size() / batch_size);
		if (quit) break; // still need to send this signal to the inner functions

		nn.test(test_data);

	}

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	// launch compute thread
	std::thread compute_thread(compute);

	try {

		nanogui::init();

		/* scoped variables */ {
			nanogui::ref<Manifold> app = new Manifold();
			app->drawAll();
			app->setVisible ( true );
			nanogui::mainloop ( 1 );
		}

		quit = true;

		nanogui::shutdown();
		compute_thread.join();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
#if defined(_WIN32)
		MessageBoxA ( nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK );
#else
		std::cerr << error_msg << std::endl;
#endif
		return -1;
	}

	return 0;

}
