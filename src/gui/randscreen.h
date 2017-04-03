#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/entypo.h>

// helpers
#include <utils.h>
#include <colors.h>

#include "aux.h"
#include "perf.h"
#include "fps.h"

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glrand.h>
#include <gui/glplothelper.h>

#include <gl/tex.h>

#define DEF_WIDTH 1300
#define DEF_HEIGHT 720
#define SCREEN_NAME "Rand"
#define RESIZABLE true
#define FULLSCREEN false
#define COLOR_BITS 8
#define ALPHA_BITS 8
#define DEPTH_BITS 24
#define STENCIL_BITS 8
#define MSAA_SAMPLES 4
#define GL_MAJOR 3
#define GL_MINOR 3
#define VSYNC true
#define AUTOSIZE false
#define SIZE_RATIO 2.0f/3.0f

class GUI : public nanogui::Screen {

  public:

	GUI ( ) :

		nanogui::Screen ( { DEF_WIDTH , DEF_HEIGHT  }, SCREEN_NAME, RESIZABLE, FULLSCREEN, COLOR_BITS, ALPHA_BITS, DEPTH_BITS, STENCIL_BITS, MSAA_SAMPLES, GL_MAJOR, GL_MINOR, VSYNC, AUTOSIZE, SIZE_RATIO) { init(); }

	void init() {

		// get GL capabilities info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		// init data
		plot_data = new PlotData();

		float box_size = 10.0f;

		generate_cube(plot_data->c_indices, plot_data->c_vertices, plot_data->c_colors, {0, 0, 0}, box_size, {0.5f, 0.5f, 0.5f});

		plot_data->updated();

		// gui elements
		root = new nanogui::Window ( this, "" );
		root->setLayout ( new nanogui::VGroupLayout(5) );

		std::string t = return_current_time_and_date("%y%m%d_%H%M%S");

		plots.push_back(new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, true, true, true, false, mGLFWWindow, mNVGContext, 80.0f, { -15.2f, -15.8f, -15.0f}, { 0.63, 1.895f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot0", t.c_str())));
		// plots.push_back(new Plot ( root, "front", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, true, true, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot_ortho", t.c_str())));

		plots.push_back(new Plot ( root, "top frustum", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 1, plot_data, true, false, false, true, mGLFWWindow, mNVGContext, 66.0f, {0.0f, 35.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot_top", t.c_str())));
		plots.push_back(new Plot ( root, "front ortho", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, true, false, false, false, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, true, 0, string_format ( "%s_plot_ortho", t.c_str())));

		int number_of_cameras = plots.size();

		plot_data->e_vertices.resize(3, 2 * number_of_cameras);
		plot_data->r_vertices.resize(3, 2 * number_of_cameras);
		plot_data->e_colors.resize(3, 2 * number_of_cameras);

		plot_data->e_colors.col(0) << 1, 0, 0; plot_data->e_colors.col(1) << 1, 0, 0;
		plot_data->e_colors.col(2) << 0, 1, 0; plot_data->e_colors.col(3) << 0, 1, 0;
		plot_data->e_colors.col(4) << 0, 0, 1; plot_data->e_colors.col(5) << 0, 0, 1;

		// share shader data
		for (int i = 0; i < number_of_cameras; i++) {
			plots[i]->master_pointShader = plots[0]->m_pointShader;
		}

		// todo: set/save layout (including dynamically created widgets)
		// be able to load everything

		drawAll();
		setVisible ( true );

		/* widgets */
		makeWidgets();

		performLayout();

		resizeEvent ( {DEF_WIDTH, DEF_HEIGHT} );

		// DATA
		// train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
		// test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

		update_data_textures();

	}

	void saveScreenShot(int premult, const char* name) {

		int w = mFBSize[0];
		int h = mFBSize[1];

		screenshot(0, 0, w, h, premult, name);

	}

	void saveScreenShotCropped(int premult, const char* name) {

		int w = mFBSize[0];
		int h = mFBSize[1];
		int x = 0;
		int y = h - w / 3;
		h = w / 3;
		w = w / 3;

		screenshot(x, y, w, h, premult, name);

	}

	virtual void drawContents() { }

	/* event handlers */
	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		//todo: logf(type, "...", );

		printf ( "keyboardEvent: key %i scancode %i (%s), action %i mods %i\n", key, scancode, glfwGetKeyName ( key, scancode ), action, modifiers );

		if ( action ) {

			switch ( key ) {

			case GLFW_KEY_ESCAPE:

				saveScreenShotCropped(false, "dump.png");
				setVisible ( false );
				return true;

			case GLFW_KEY_F12:

				std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
				saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
				return true;

			}

		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual void frame_completed_event() {

		int interval = 250;

		if (completed_frames % interval == 0) {
			std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
			// saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
		}

		completed_frames++;

	};

	void update_data_textures() {

		// plot_data->load_data_textures(train_data, mNVGContext);

	}

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		// 8 because of VGroupLayout(5) spacing
		for (int i = 0; i < (int) plots.size(); i++) {
			plots[i]->setSize({size[0] / 3 - 8, size[0] / 3 - 8});
			plots[i]->resizeEvent({size[0] / 3 - 8, size[0] / 3 - 8});
		}

		performLayout();

		window->setPosition(Eigen::Vector2i(5, size[0] / 3 + 8));
		graphs->setPosition(Eigen::Vector2i(window->size()[0] + 12, size[0] / 3 + 8));
		// controls->setPosition(Eigen::Vector2i(window->size()[0] + graphs->size()[0] + 8, size[0] / 3 + 5));
		// needs to be called 2nd time
		performLayout();

		return true;

	}

	void makeWidgets() {

		nanogui::GridLayout *layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 15, 5);
		layout->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout->setSpacing(0, 10);

		nanogui::GridLayout *layout_2cols = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 15, 5);
		layout_2cols->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout_2cols->setSpacing(0, 10);


		// nanogui::Label* message = new nanogui::Label(window, "", "sans-bold");

		window = new nanogui::Window(this, "");
		window->setPosition(Eigen::Vector2i(3, DEF_WIDTH / 3 + 3));
		window->setLayout(layout);

		graphs = new nanogui::Window(this, "");
		graphs->setPosition(Eigen::Vector2i(window->size()[0] + 12, DEF_WIDTH / 3 + 5));
		graphs->setLayout(layout);

		graph_loss = new nanogui::Graph ( graphs, "" );
		graph_loss->setFooter ( "loss" );
		graph_loss->setGraphColor ( nanogui::Color ( 128, 128, 128, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_loss->values().resize(500);
		graph_loss->values().setZero();

		graph_cpu = new nanogui::Graph ( graphs, "" );
		graph_cpu->setFooter ( "cpu ms per iter" );
		graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_cpu->values().resize(500);
		graph_cpu->values().setZero();

		// FlOP/s
		graph_flops = new nanogui::Graph ( graphs, "" );
		graph_flops->setFooter ( "GFLOP/s" );
		graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_flops->values().resize(500);
		graph_flops->values().setZero();

		// B/s
		graph_bytes = new nanogui::Graph ( graphs, "" );
		graph_bytes->setFooter ( "MB/s" );
		graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_bytes->values().resize(500);
		graph_bytes->values().setZero();

		// /* FPS GRAPH */
		// graph_fps = new nanogui::Graph ( graphs, "" );
		// graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		// graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_fps->values().resize(500);
		// graph_fps->values().setZero();

		//legend
		nanogui::Graph *legend = new nanogui::Graph ( graphs, "", nanogui::GraphType::GRAPH_LEGEND );
		legend->values().resize ( 10 );
		legend->values() << 1, 1, 1, 1, 1, 1, 1, 1, 1, 1;

		/* widgets end */


	}

	~GUI() { /* free resources */ }

	std::vector<Plot*> plots;
	PlotData *plot_data;
	nanogui::Window *root;
	nanogui::Window *window;
	nanogui::Window *graphs;
	nanogui::Window *controls;
	nanogui::Graph *graph_loss;
	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;

	int completed_frames = 0;

	size_t local_data_checksum = 0;

	using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
	imagesDataType mImagesData;
	int mCurrentImage;

};

#endif
