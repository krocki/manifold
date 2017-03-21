#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>

// helpers
#include <utils.h>

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glplot.h>

#define DEF_WIDTH 960
#define DEF_HEIGHT 700
#define SCREEN_NAME "Manifold"
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
		generate_cube(plot_data->c_indices, plot_data->c_vertices, plot_data->c_colors, {0, 0, 0}, 10.0f, {0.8f, 0.8f, 0.8f});
		plot_data->updated();

		// gui elements
		nanogui::Window *window = new nanogui::Window ( this, "" );
		nanogui::GridLayout *gridlayout = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 2, 2 );
		gridlayout->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		gridlayout->setSpacing ( 0, 5 );

		window->setLayout ( gridlayout );

		plots.push_back(new Plot ( window, {350, 350}, 0, plot_data, true, mGLFWWindow));
		plots.push_back(new Plot ( window, {350, 350}, 1, plot_data, true, nullptr, 40.0f, {0.0f, 65.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}));
		plots.push_back(new Plot ( window, {350, 350}, 2, plot_data, true, nullptr, 60.0f, {0.0f, 0.0f, 30.0f}, {0.0f, 0.0f, 0}));
		plots.push_back(new Plot ( window, {350, 350}, 3, plot_data, true, nullptr, 60.0f, {30.0f, 0.0f, 0.0f}, {0.0f, M_PI / 4.0f, 0.0f}));

		int number_of_cameras = plots.size();
		plot_data->e_vertices.resize(3, 2 * number_of_cameras);
		plot_data->e_colors.resize(3, 2 * number_of_cameras);

		// todo: set/save layout (including dynamically created widgets)
		// be able to load everything

		drawAll();
		setVisible ( true );

		performLayout();

	}


	virtual void drawContents() { }

	/* event handlers */
	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		//todo: logf(type, "...", );

		printf ( "keyboardEvent: key %i scancode %i (%s), action %i mods %i\n", key, scancode, glfwGetKeyName ( key, scancode ), action, modifiers );

		if ( action ) {

			switch ( key ) {

			case GLFW_KEY_ESCAPE:

				setVisible ( false );
				return true;

			}
		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		UNUSED ( size );
		performLayout();

		return true;

	}

	~GUI() { /* free resources */}

	std::vector<Plot*> plots;
	PlotData *plot_data;

};

#endif