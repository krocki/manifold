/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-09 22:24:04
*/

#include <thread>
#include <unistd.h>

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/layout.h>

//helpers
#include <utils.h>

//for NN
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

NN* nn;

#define DEF_WIDTH 300
#define DEF_HEIGHT 400
#define SCREEN_NAME "AE"

class GUI : public nanogui::Screen {

  public:

	GUI ( ) : nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ) { init(); }

	void init() {

		/* get physical GLFW screen size */
		glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

		// get GL capabilities info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		/* * create widgets  * */
		this->setLayout(new nanogui::BoxLayout());

		nanogui::Window* window_graphs = new nanogui::Window ( this, "" );
		window_graphs->setLayout(new nanogui::GroupLayout(3, 1, 0, 0));

		int NUM_GRAPHS = 10;

		for (int i = 0; i < NUM_GRAPHS; i++) {

			nanogui::Graph* graph = new nanogui::Graph ( window_graphs, string_format("%d", i),
			        nanogui::GraphType::GRAPH_NANOGUI_NOFILL, nanogui::parula_lut[i] );

			graph_data.push_back(graph->values_ptr());
		}

		window_graphs->setSize({glfw_window_width, glfw_window_height});
		/* * * * * * * * * * * */

		drawAll();
		setVisible(true);

		glfwSwapInterval(vsync);

		performLayout();
		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}


	virtual void drawContents() {

		refresh();

	}

	void refresh() {

		for (size_t i = 0; i < graph_data.size(); i++) {

			if (graph_data[i]) {

				graph_data[i]->resize(100);

				for (int k = 0; k < graph_data[i]->size(); ++k) {

					graph_data[i]->operator[](k) = 0.5f * (0.5f * std::sin(k / 10.f + glfwGetTime()) +
					                                       0.5f * std::cos(i * k / 23.f) + 1);

				}
			}
		}
	}

	/* event handlers */

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		/* process subcomponents */
		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		/* close */
		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {

			nn->quit = true;
			setVisible ( false );

			return true;
		}

		return false;
	}

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		UNUSED(size);
		performLayout();

		return true;

	}

	~GUI() { /* free resources */}

  protected:

	int glfw_window_width, glfw_window_height;
	bool vsync;

	std::vector<Eigen::VectorXf*> graph_data;

};

GUI* screen;

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

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;

}
