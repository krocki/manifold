/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-06 22:17:30
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
#include <nanogui/imageview.h>
#include <nanogui/imagepanel.h>
#include <nanogui/checkbox.h>

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

#define DEF_WIDTH 640
#define DEF_HEIGHT 480
#define SCREEN_NAME "vae"

bool quit = false;

NN* nn;

#define N 250
GLuint mTextureId[N];
int w = 28;
int h = 28;

int nvgCreateImageA(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data) {

	return nvgInternalParams(ctx)->renderCreateTexture(nvgInternalParams(ctx)->userPtr, NVG_TEXTURE_ALPHA, w, h, imageFlags, data);
}

class Manifold : public nanogui::Screen {

  public:

	Manifold ( ) : nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ) {

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

		//NN loss graph
		graph_loss = new nanogui::Graph ( this, "" );
		graph_loss->setGraphColor ( nanogui::Color ( 160, 160, 160, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );

		graphs = new nanogui::Window ( this, "" );
		graphs->setPosition({5, size()[1] - graphs->size()[1] - 5});
		graphs->setPosition ( {5, size()[1] - graphs->size()[1] - 5} );
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

		//inputs checkbox
		m_showInputsCheckBox = new nanogui::CheckBox(this, "show xs");
		m_showInputsCheckBox->setCallback([&](bool) { });
		m_showInputsCheckBox->setPosition( {30, 30 } );

		show_console = false;

		console_window = new nanogui::Window ( this, "" );

		// console_window->setLayout ( new GroupLayout ( 5, 5, 0, 0 ) );
		console_panel = new nanogui::Console ( console_window );
		console_panel->setFontSize ( 12 );

		// debug footer message
		footer_message = new nanogui::Console ( this );
		footer_message->setFontSize ( 12 );
		footer_message_string = "";

		glGenTextures(N, mTextureId);

		m_showInputsCheckBox->setChecked(true);

		rgba_image.resize(w, h);

		for (size_t i = 0; i < N; i++) {

			int im = nvgCreateImageA(nvgContext(), w, h, NVG_IMAGE_NEAREST, (unsigned char*) rgba_image.data());
			mImagesData.emplace_back(std::pair<int, std::string>(im, ""));

		}

		auto imageWindow2 = new nanogui::Window(this, "");
		imageWindow2->setPosition(Eigen::Vector2i(120, 15));
		imageWindow2->setSize(Eigen::Vector2i(260, 400));
		imgPanel = new nanogui::ImagePanel(imageWindow2, 16, 3, 5);
		imgPanel->setSize(Eigen::Vector2i(260, 400));
		imgPanel->setImages(mImagesData);

		drawAll();
		setVisible(true);

		resizeEvent ( { glfw_window_width, glfw_window_height } );

		performLayout();
	}

	~Manifold() { }

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {
		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			setVisible ( false ); quit = true;
			return true;
		}

		if ( key == GLFW_KEY_TAB && action == GLFW_PRESS ) {

			show_console = !show_console;
			console_window->setVisible ( show_console );

		}

		return false;
	}

	void draw_inputs() {

		if (nn) {

			for (size_t i = 0; i < N; i++) {

				Eigen::MatrixXf float_image = nn->layers[0]->x.col(i);
				float_image.resize(w, h);

				for (int y = 0; y < h; ++y) {
					for (int x = 0; x < w; ++x) {

						unsigned char color = (unsigned char) (float_image(x, y) * 255.0f);
						rgba_image(x, y) = color;
						//if RGBA -> rgba_image(x, y) = ((color << 24) | (color << 16) | (color << 8) | (0xFF << 0));
					}
				}

				nvgUpdateImage(nvgContext(), mImagesData[i].first, (unsigned char*) rgba_image.data());

			}
		}
	}

	void drawContents() {

		graph_cpu->values() = Eigen::Map<Eigen::VectorXf> ( cpu_util.data(), cpu_util.size() );
		graph_flops->values() = Eigen::Map<Eigen::VectorXf> ( cpu_flops.data(), cpu_flops.size() );

		char str[16];
		int last_avg = 10;

		sprintf ( str, "%5.1f ms\n", 1000.0 * graph_cpu->values().block ( cpu_util.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );
		graph_cpu->setHeader ( str );

		sprintf ( str, "%5.1f GF/s\n", graph_flops->values().block ( cpu_flops.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );
		graph_flops->setHeader ( str );

		console_window->setVisible(show_console);
		console_panel->setValue ( log_str );

		// debug
		std::stringstream ss;
		ss << size() [0] << "x" << size() [1] << "\n";

		footer_message_string = ss.str();
		footer_message->setValue ( footer_message_string );

		if (m_showInputsCheckBox->checked()) {

			imgPanel->setVisible(true);
			draw_inputs();

		} else {

			imgPanel->setVisible(false);
		}

		update_FPS(graph_fps);

		if (nn)
			if (nn->clock) {
				nn->clock = false;
				update_graph (graph_loss, nn->current_loss );
			}

	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		// FPS graph
		graphs->setPosition ( {1, size[1] - 50} );

		// loss
		graph_loss->setSize ( {size[0] - 108, 45 } );
		graph_loss->setPosition( {105, size[1] - 48 } );

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

	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_loss;
	nanogui::Window *console_window, *graphs;
	nanogui::Console *console_panel, *footer_message;
	nanogui::CheckBox *m_showInputsCheckBox;

	using imagesDataType = std::vector<std::pair<int, std::string>>;
	imagesDataType mImagesData;
	nanogui::ImagePanel * imgPanel;

	std::string log_str, footer_message_string;

	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

	bool show_console;

};

int compute() {

	// TODO: option to suspend or kill this thread from GUI

	size_t epochs = 100;
	size_t batch_size = 250;
	double learning_rate = 1e-3;

	nn = new NN(batch_size);

	nn->layers.push_back(new Linear(28 * 28, 256, batch_size));
	nn->layers.push_back(new ReLU(256, 256, batch_size));
	nn->layers.push_back(new Linear(256, 256, batch_size));
	nn->layers.push_back(new ReLU(256, 256, batch_size));
	nn->layers.push_back(new Linear(256, 100, batch_size));
	nn->layers.push_back(new ReLU(100, 100, batch_size));
	nn->layers.push_back(new Linear(100, 10, batch_size));
	nn->layers.push_back(new Softmax(10, 10, batch_size));

	//[60000, 784]
	std::deque<datapoint> train_data =
	    MNISTImporter::importFromFile("data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte");
	//[10000, 784]
	std::deque<datapoint> test_data =
	    MNISTImporter::importFromFile("data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte");

	for (size_t e = 0; e < epochs; e++) {

		std::cout << "Epoch " << e + 1 << std::endl << std::endl;
		nn->train(train_data, learning_rate, train_data.size() / batch_size);
		if (quit) break; // still need to send this signal to the inner functions

		nn->test(test_data);

	}

	delete nn;

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		// launch a compute thread
		std::thread compute_thread(compute);

		nanogui::init();

		Manifold* screen = new Manifold();
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
