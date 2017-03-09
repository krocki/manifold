/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-08 20:57:59
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
#define CONSOLE_MODE_WIDTH 280
#define CONSOLE_MODE_HEIGHT 540
#define MINI_MODE_WIDTH 280
#define MINI_MODE_HEIGHT 95
#define SCREEN_NAME "vae"

NN* nn;

#define N 16
#define L 100

int w = 28;
int h = 28;

class Manifold : public nanogui::Screen {

  public:

	Manifold ( ) :
		nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ) { init(); }

	void console ( const char *pMsg, ... ) {

		/*
			TODO:
			- split TIME | MESSAGE
			- add colors
			- allow input, some simple debugging commands
			- vscroll
		*/

		char buffer[4096];
		std::va_list arg;
		va_start ( arg, pMsg );
		std::vsnprintf ( buffer, 4096, pMsg, arg );
		va_end ( arg );
		log_str.append( "[" + to_string_with_precision(time_since_start()) + "]   ");
		log_str.append ( buffer );

	}

	void init() {

		/*
			TODO:
			- move most this code somewhere else
		*/

		console ( "init()\n");

		int glfw_window_width, glfw_window_height;

		glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

		// get GL debug info
		console ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		console ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		console ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

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

		// debug footer message
		footer_message = new nanogui::Label ( chk_windows, "" );

		//inputs checkbox
		fpslimit = new nanogui::CheckBox(chk_windows, "vsync");
		fpslimit->setCallback([&](bool) { glfwSwapInterval(fpslimit->checked()); });

		m_showInputsCheckBox = new nanogui::CheckBox(chk_windows, "inputs");
		m_showInputsCheckBox->setCallback([&](bool) { });

		m_showOutputsCheckBox = new nanogui::CheckBox(chk_windows, "outputs");
		m_showOutputsCheckBox->setCallback([&](bool) { });

		m_showWeightsCheckBox = new nanogui::CheckBox(chk_windows, "net");
		m_showWeightsCheckBox->setCallback([&](bool) { });

		m_showConsole = new nanogui::CheckBox(chk_windows, "console");
		m_showConsole->setCallback([&](bool) { });

		while (!nn) { std::cout << "Waiting for compute thread...\n"; usleep(10000); }

		console ( "Sync point 1: GUI Window created\n");

		rgba_image.resize(w, h);

		for (size_t i = 0; i < N; i++) {

			int im = nvgCreateImageA(nvgContext(), w, h, NVG_IMAGE_NEAREST, (unsigned char*) nullptr);
			mImagesData.emplace_back(std::pair<int, std::string>(im, ""));

		}

		for (size_t i = 0; i < L; i++) {

			layer_image[i].resize(w, h);
			int l_im = nvgCreateImageA(nvgContext(), w, h, NVG_IMAGE_NEAREST, (unsigned char*) nullptr);
			layerImagesData.emplace_back(std::pair<int, std::string>(l_im, ""));

		}

		inputs = new nanogui::Window(this, "inputs");
		inputs->setPosition(Eigen::Vector2i(15, 35));
		imgPanel = new nanogui::ImagePanel(inputs, 45, 3, 5, {4, 4});
		imgPanel->setImages(mImagesData);
		imgPanel->setPosition({0, 20});

		outputs = new nanogui::Window(this, "outputs");
		outputs->setPosition(Eigen::Vector2i(225, 35));

		nanogui::GridLayout *outputslayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 3, 3);
		outputslayout->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		outputslayout->setSpacing(0, 3);

		outputs->setLayout(outputslayout);

		for (size_t i = 0; i < N; i++) {

			graph_output[i] = new nanogui::Graph(outputs, "", nanogui::GraphType::GRAPH_COLORBARS);
			graph_output[i]->setSize ( {45, 45 } );
			graph_output[i]->setGraphColor ( nanogui::Color ( 255, 0, 0, 255 ) );
			graph_output[i]->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
			graph_output[i]->mFill = true;

		}

		Eigen::Vector2i w_size = imgPanel->preferredSize() + Eigen::Vector2i({0, 20});
		inputs->setSize(w_size);
		nanogui::Theme *th = inputs->theme();
		th->mWindowFillUnfocused = nanogui::Color ( 0, 0, 0, 32 );
		th->mWindowFillFocused = nanogui::Color ( 32, 32, 32, 32 );
		th->mWindowHeaderSepTop = nanogui::Color ( 0, 0, 0, 32 );
		th->mWindowHeaderHeight = 22;
		inputs->setTheme ( th );
		outputs->setTheme ( th );

		nn_window = new nanogui::Window(this, "Layers");
		nn_window->setPosition({440, 35 });
		nn_imgPanel = new nanogui::ImagePanel(nn_window, 46, 2, 4, {10, 10});
		nn_imgPanel->setImages(layerImagesData);
		nn_imgPanel->setPosition({0, 20});
		nanogui::Theme *th_nn = nn_imgPanel->theme();
		th_nn->mWindowFillUnfocused = nanogui::Color ( 0, 0, 0, 32 );
		th_nn->mWindowFillFocused = nanogui::Color ( 0, 0, 0, 128 );;
		nn_imgPanel->setTheme(th_nn);

		w_size = nn_imgPanel->preferredSize() + Eigen::Vector2i({0, 20});
		nn_window->setSize(w_size);

		console_window = new nanogui::Window ( this, "" );
		console_window->setLayout(new nanogui::GroupLayout());

		//TODO
		nanogui::VScrollPanel *vscroll = new nanogui::VScrollPanel(console_window);

		console_panel = new nanogui::Console ( vscroll );
		console_panel->setFontSize ( 12 );

		footer_message->setFontSize ( 12 );
		footer_message_string = "";

		fpslimit->setChecked(true);
		fpslimit->setFontSize ( 12 );
		m_showConsole->setChecked(true);
		m_showConsole->setFontSize ( 12 );
		m_showInputsCheckBox->setChecked(true);
		m_showOutputsCheckBox->setFontSize ( 12 );
		m_showOutputsCheckBox->setChecked(true);
		m_showInputsCheckBox->setFontSize ( 12 );
		m_showWeightsCheckBox->setChecked(true);
		m_showWeightsCheckBox->setFontSize ( 12 );

		drawAll();
		setVisible(true);

		glfwSwapInterval(fpslimit->checked());

		console ( "GUI init completed\n");

		console_mode = false;
		mini_mode = false;
		performLayout();

		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}

	~Manifold() { }

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			setVisible ( false ); nn->quit = true;
			return true;
		}

		if ( key == GLFW_KEY_TAB && action == GLFW_PRESS ) {

			m_showConsole->setChecked(!m_showConsole->checked());

		}

		if ( key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			nn->pause = !nn->pause;
		}

		if ( key == GLFW_KEY_SLASH && action == GLFW_PRESS) {

			//full view
			console_mode = false;
			mini_mode = false;
			setSize(prev_size);

		}

		if ( key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {

			//console view
			console_mode = true;
			mini_mode = false;
			setSize({chk_windows->size()[0] + 10, CONSOLE_MODE_HEIGHT});

		}

		if ( key == GLFW_KEY_COMMA && action == GLFW_PRESS) {

			// mini view
			console_mode = false;
			mini_mode = true;
			setSize({chk_windows->size()[0] + 10, MINI_MODE_HEIGHT});

		}

		if ( key == GLFW_KEY_N && action == GLFW_PRESS) {

			if (nn->pause) {
				nn->step = true;
			}

		}


		return false;
	}

	void refresh_outputs() {

		if (nn) {

			for (size_t i = 0; i < N; i++) {

				graph_output[i]->values() = nn->layers.back()->y.col(i);

			}
		}
	}

	void refresh_inputs() {

		if (nn) {

			for (size_t i = 0; i < N; i++) {

				Eigen::MatrixXf float_image = nn->layers[0]->x.col(i);

				float_image.resize(w, h);
				float_image *= 255.0f;

				rgba_image = float_image.cast<unsigned char>();
				nvgUpdateImage(nvgContext(), mImagesData[i].first, (unsigned char*) rgba_image.data());

			}
		}
	}

	void refresh_layers() {

		if (nn) {

			for (size_t i = 0; i < 1; i++) {

				for (size_t j = 0; j < L; j++) {

					Eigen::MatrixXf float_image = ((Linear*)(nn->layers[i]))->W.row(j);
					float l2 = float_image.norm();
					float_image = float_image.unaryExpr([&](float x) { return 255.0f * (x / l2 + 0.5f); });
					layer_image[j] = float_image.cast<unsigned char>();
					nvgUpdateImage(nvgContext(), layerImagesData[j].first, (unsigned char*) layer_image[j].data());

				}

			}
		}
	}

	void drawContents() {

		// always visible
		// checkboxes, graphs
		// + debug message
		char str[64];
		sprintf(str, "%04dx%04d", size() [0], size() [1]);
		//TODO: make formatted setCaption like console
		footer_message->setCaption ( str );

		if (!mini_mode) {

			console_window->setVisible(true);
			console_panel->setValue ( log_str );

			if (console_mode) {

				nn_window->setVisible(false);
				inputs->setVisible(false);
				outputs->setVisible(false);

			} else {

				nn_window->setVisible(m_showWeightsCheckBox->checked());
				inputs->setVisible(m_showInputsCheckBox->checked());
				outputs->setVisible(m_showOutputsCheckBox->checked());

			}

		} else {

			console_window->setVisible(false);
			nn_window->setVisible(false);
			inputs->setVisible(false);
			outputs->setVisible(false);

		}

		if (!(nn->pause) || (nn->step)) {

			refresh_inputs();
			refresh_outputs();
			refresh_layers();

		}

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

		// FPS graph
		graphs->setPosition ( {1, size[1] - 70} );

		// loss
		graph_loss->setSize ( {size[0] - 109, graphs->size()[1] } );
		graph_loss->setPosition( {105, graphs->position()[1] });

		// console
		int console_width = 250;
		int console_height = size[1] - 100;

		if (console_mode) {

			console_height = size[1] - 120;
			console_width = size[0] - 32;
			console_window->setPosition ( {5, 25} );

		} else {

			console_window->setPosition ( {size[0] - console_width - 27, 5} );

			if (!mini_mode)
				prev_size = size;
		}

		console_window->setSize ( {console_width, console_height} );

		console_panel->setPosition ( {0, 0} );
		console_panel->setWidth ( console_width - 20 );
		console_panel->setHeight ( console_height - 10 );

		// footer
		int footer_height = 40;
		int footer_width = 550;

		footer_message->setPosition ( {5, 5} );
		footer_message->setSize ( {footer_width, footer_height} );

		performLayout();

		return true;

	}

	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes, *graph_loss, *graph_output[N];
	nanogui::Window *console_window, *graphs, *nn_window, *inputs, *outputs, *chk_windows;
	nanogui::Console *console_panel;
	nanogui::Label *footer_message;

	nanogui::CheckBox *fpslimit, *m_showOutputsCheckBox, *m_showInputsCheckBox, *m_showWeightsCheckBox, *m_showConsole;

	using imagesDataType = std::vector<std::pair<int, std::string>>;
	imagesDataType mImagesData, layerImagesData;
	nanogui::ImagePanel *imgPanel, *nn_imgPanel;

	std::string log_str, footer_message_string;

	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> layer_image[L];

	Eigen::Vector2i prev_size;

	bool console_mode;
	bool mini_mode;
};

Manifold* screen;

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization

	size_t batch_size = 16;
	double learning_rate = 1e-3;
	float decay = 1e-5;
	nn = new NN(batch_size, decay);

	nn->layers.push_back(new Linear(28 * 28, 100, batch_size));
	nn->layers.push_back(new ReLU(100, 100, batch_size));
	// nn->layers.push_back(new Linear(100, 100, batch_size));
	// nn->layers.push_back(new ReLU(100, 100, batch_size));
	nn->layers.push_back(new Linear(100, 10, batch_size));
	nn->layers.push_back(new Softmax(10, 10, batch_size));

	while (!screen) { usleep(1000); }

	screen->console ( "Sync point 2: NN init completed\n");
	screen->console ( "nn->layers.size() = %d\n\n", nn->layers.size());

	//[60000, 784]
	std::deque<datapoint> train_data =
	    MNISTImporter::importFromFile("data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte");

	//[10000, 784]
	std::deque<datapoint> test_data =
	    MNISTImporter::importFromFile("data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte");

	screen->console ( "Data loaded (%d/%d datapoints)\n\n", train_data.size(), test_data.size());

	for (size_t e = 0; true; e++) {

		std::cout << "Epoch " << e + 1 << std::endl << std::endl;
		nn->train(train_data, learning_rate, train_data.size() / batch_size);
		if (nn->quit) break;

		float acc = (float)nn->test(test_data);

		if (screen)
			screen->console ( "Epoch %3d: %.2f %%\n", e + 1, 100.0f * acc );

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

		screen = new Manifold();
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
