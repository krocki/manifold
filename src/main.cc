/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-12 20:17:33
*/

#include <thread>
#include <unistd.h>

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/layout.h>
#include <nanogui/imagepanel.h>

//helpers
#include <utils.h>

//for NN
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

// nvgCreateImageA
#include <gl/tex.h>

NN* nn;

#define DEF_WIDTH 1066
#define DEF_HEIGHT 607
#define SCREEN_NAME "AE"

const size_t batch_size = 64;
const size_t image_size = 28;

class GUI : public nanogui::Screen {

  public:

	GUI ( ) : nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ), vsync(true) { init(); }

	void init() {

		/* get physical GLFW screen size */
		glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

		// get GL capabilities info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		/* * create widgets  * */

		//make image placeholders
		for (size_t i = 0; i < batch_size; i++) {

			xs.emplace_back(std::pair<int, std::string>(nvgCreateImageA(nvgContext(),
			                image_size, image_size, NVG_IMAGE_NEAREST, (unsigned char*) nullptr), ""));

			ys.emplace_back(std::pair<int, std::string>(nvgCreateImageA(nvgContext(),
			                image_size, image_size, NVG_IMAGE_NEAREST, (unsigned char*) nullptr), ""));

		}

		this->setLayout(new nanogui::BoxLayout());

		nanogui::Window* images = new nanogui::Window ( this, "images" );
		images->setLayout(new nanogui::GroupLayout(3, 1, 0, 0));

		nanogui::ImagePanel* inp = new nanogui::ImagePanel(images, 64, 2, 2, {16, image_size / 16});
		inp->setImages(xs);
		nanogui::ImagePanel* out = new nanogui::ImagePanel(images, 64, 2, 2, {16, image_size / 16});
		out->setImages(ys);

		images->setSize({glfw_window_width, glfw_window_height});
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

		if (nn) {

			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;
			rgba_image.resize(image_size, image_size);

			// xs
			for (size_t i = 0; i < nn->batch_size; i++) {

				if (nn->layers[0]) {

					Eigen::MatrixXf float_image = nn->layers[0]->x.col(i);

					float_image.resize(image_size, image_size);
					float_image *= 255.0f;

					rgba_image = float_image.cast<unsigned char>();
					nvgUpdateImage(nvgContext(), xs[i].first, (unsigned char*) rgba_image.data());

				}

			}

			// ys
			for (size_t i = 0; i < nn->batch_size; i++) {

				if (nn->layers[7]) {

					Eigen::MatrixXf float_image = nn->layers.back()->y.col(i);

					float_image.resize(image_size, image_size);
					float_image *= 255.0f;

					rgba_image = float_image.cast<unsigned char>();
					nvgUpdateImage(nvgContext(), ys[i].first, (unsigned char*) rgba_image.data());

				}
			}

		}

	}

	/* event handlers */

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		/* process subcomponents */
		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			nn->pause = !nn->pause;
		}

		if ( key == GLFW_KEY_N && action == GLFW_PRESS) {

			if (nn->pause) {
				nn->step = true;
			}

		}

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

		printf("%d %d\n", size[0], size[1]);
		return true;

	}

	~GUI() { /* free resources */}

  protected:

	int glfw_window_width, glfw_window_height;
	bool vsync;

	std::vector<Eigen::VectorXf*> graph_data;

	using imagesDataType = std::vector<std::pair<int, std::string>>;
	imagesDataType xs, ys;

};

GUI* screen;

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization

	double learning_rate = 1e-3;
	float decay = 0;//1e-7;
	nn = new NN(batch_size, decay, DAE);

	nn->layers.push_back(new Linear(image_size * image_size, 256, batch_size));
	nn->layers.push_back(new ReLU(256, 256, batch_size));

	nn->layers.push_back(new Linear(256, 3, batch_size));
	nn->layers.push_back(new ReLU(3, 3, batch_size));

	nn->layers.push_back(new Linear(3, 256, batch_size));
	nn->layers.push_back(new ReLU(256, 256, batch_size));

	nn->layers.push_back(new Linear(256, image_size * image_size, batch_size));
	nn->layers.push_back(new Sigmoid(image_size * image_size, image_size * image_size, batch_size));

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

		printf ( "Epoch %3lu: Loss: %.2f\n", e + 1, (float)nn->test(test_data));

		// test - go over some data, plot
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
