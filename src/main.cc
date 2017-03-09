/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-09 11:06:03
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <thread>
#include <unistd.h>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/theme.h>
#include <nanogui/graph.h>
#include <nanogui/matrix.h>
#include <nanogui/console.h>
#include <nanogui/imageview.h>
#include <nanogui/imagepanel.h>
#include <nanogui/checkbox.h>
#include <nanogui/vscrollpanel.h>

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
#include <gl/tex.h>

#define DEF_WIDTH 1250
#define DEF_HEIGHT 620
#define SCREEN_NAME "ae"

NN* nn;

class AE : public nanogui::Screen {

  public:

	AE ( ) :
		nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ) { init(); }

	void init() {

		/*
			TODO:
			- move most this code somewhere else
		*/

		int glfw_window_width, glfw_window_height;

		glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

		// get GL debug info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		//NN loss graph
		graph_loss = new nanogui::Graph ( this, "" );
		graph_loss->setGraphColor ( nanogui::Color ( 160, 160, 160, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );

		graphs = new nanogui::Window ( this, "" );
		graphs->setPosition({5, size()[1] - graphs->size()[1] - 5});
		nanogui::GridLayout *layout = new nanogui::GridLayout(
		    nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 1, 1);
		layout->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout->setSpacing(0, 5);
		graphs->setLayout(layout);
		nanogui::Theme *t = graphs->theme();

		t->mWindowFillUnfocused = nanogui::Color (0, 0, 0, 0 );
		t->mWindowFillFocused = nanogui::Color (128, 128, 128, 16 );
		t->mDropShadow = nanogui::Color (0, 0, 0, 0 );
		graphs->setTheme ( t );

		int graph_width = 100;
		int graph_height = 15;

		//FPS graph
		graph_fps = graphs->add<nanogui::Graph> ( "" );
		graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
		graph_fps->setSize ( {graph_width, graph_height } );

		//CPU graph
		graph_cpu = graphs->add<nanogui::Graph> ( "" );
		graph_cpu->values().resize ( cpu_util.size() );
		graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
		graph_cpu->setSize ( {graph_width, graph_height } );

		// FlOP/s
		graph_flops = graphs->add<nanogui::Graph> ( "" );
		graph_flops->values().resize ( cpu_flops.size() );
		graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
		graph_flops->setSize ( {graph_width, graph_height } );

		// FlOP/s
		graph_bytes = graphs->add<nanogui::Graph> ( "" );
		graph_bytes->values().resize ( cpu_reads.size() );
		graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
		graph_bytes->setSize ( {graph_width, graph_height } );

		chk_windows = new nanogui::Window(this, "");
		chk_windows->setPosition({5, 5});
		nanogui::GridLayout *hlayout = new nanogui::GridLayout(
		    nanogui::Orientation::Horizontal, 6, nanogui::Alignment::Middle, 1, 1);
		hlayout->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		hlayout->setSpacing(0, 5);
		chk_windows->setLayout(hlayout);
		chk_windows->setTheme ( t );

		//inputs checkbox
		fpslimit = new nanogui::CheckBox(chk_windows, "vsync");
		fpslimit->setCallback([&](bool) { glfwSwapInterval(fpslimit->checked()); });

		while (!nn) { printf("Waiting for compute thread...\n"); usleep(10000); }

		fpslimit->setChecked(true);
		fpslimit->setFontSize ( 12 );

		drawAll();
		setVisible(true);

		glfwSwapInterval(fpslimit->checked());

		performLayout();

		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}

	~AE() { }

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			setVisible ( false ); nn->quit = true;
			return true;
		}

		if ( key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			nn->pause = !nn->pause;


		if ( key == GLFW_KEY_N && action == GLFW_PRESS) {

			if (nn->pause) {
				nn->step = true;
			}

		}


		return false;
	}

	void refresh_outputs() {

		if (nn) { }
	}

	void refresh_inputs() {

		if (nn) { }
	}

	void refresh_layers() {

		if (nn) { }
	}

	void drawContents() {

		update_FPS(graph_fps);

		if (nn)
			if (nn->clock) {
				nn->clock = false;
				update_graph (graph_loss, nn->current_loss );
				update_graph (graph_cpu, cpu_util, 1000.0f, "ms" );
				update_graph (graph_flops, cpu_flops, 1.0f, "GF/s"  );
				update_graph (graph_bytes, cpu_reads, 1.0f, "MB/s"  );
			}

	}

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		UNUSED(size);
		performLayout();
		return true;

	}

	nanogui::Graph *graph_loss, *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;
	nanogui::Window *graphs, *chk_windows;
	nanogui::CheckBox *fpslimit;

};

AE* screen;

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization

	size_t batch_size = 16;
	double learning_rate = 1e-3;
	float decay = 1e-5;
	nn = new NN(batch_size, decay);

	nn->layers.push_back(new Linear(28 * 28, 100, batch_size));
	nn->layers.push_back(new ReLU(100, 100, batch_size));
	nn->layers.push_back(new Linear(100, 10, batch_size));
	nn->layers.push_back(new Softmax(10, 10, batch_size));

	while (!screen) { usleep(1000); }

	//[60000, 784]
	std::deque<datapoint> train_data =
	    MNISTImporter::importFromFile("data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte");

	//[10000, 784]
	std::deque<datapoint> test_data =
	    MNISTImporter::importFromFile("data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte");

	for (size_t e = 0; true; e++) {

		nn->train(train_data, learning_rate, train_data.size() / batch_size);

		if (nn->quit) break;

		printf ( "Epoch %3lu: Test accuracy: %.2f %%\n", e + 1, 100.0f * (float)nn->test(test_data));

	}

	nn->quit = true;

	//should wait for GL to finish first before deleting nn
	delete nn; return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		init_start_time();

		// launch a compute thread
		std::thread compute_thread(compute);

		nanogui::init();

		screen = new AE();
		nanogui::mainloop ( 1 );

		delete screen;
		nanogui::shutdown();
		nn->quit = true;
		compute_thread.join();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;

}
