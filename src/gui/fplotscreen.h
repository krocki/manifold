#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>

// helpers
#include <utils.h>
#include <colors.h>

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glplot.h>

#define DEF_WIDTH 1300
#define DEF_HEIGHT 900
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
		float box_size = 10.0f;
		generate_cube(plot_data->c_indices, plot_data->c_vertices, plot_data->c_colors, {0, 0, 0}, box_size, {0.5f, 0.5f, 0.5f});
		generate_mesh(plot_data->m_indices, plot_data->m_vertices, plot_data->m_texcoords, plot_data->m_colors, {0, 0, -box_size}, box_size * 2);
		plot_data->updated();

		// gui elements
		nanogui::Window *window = new nanogui::Window ( this, "" );
		nanogui::GridLayout *gridlayout = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 2, 2 );
		gridlayout->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		gridlayout->setSpacing ( 0, 5 );

		window->setLayout ( gridlayout );

		plots.push_back(new Plot ( window, "forward", {400, 400}, 0, plot_data, true, mGLFWWindow, mNVGContext, 75.0f, {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}));
		plots.push_back(new Plot ( window, "top frustum", {400, 400}, 1, plot_data, false, nullptr, mNVGContext, 40.0f, {0.0f, 35.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}));
		plots.push_back(new Plot ( window, "back frustum", {400, 400}, 2, plot_data, false, nullptr, mNVGContext, 60.0f, {0.0f, 0.0f, -15.0f}, { 0.0f, -M_PI / 2.0f, 0.0f}));
		plots.push_back(new Plot ( window, "top ortho", {400, 400}, 3, plot_data, false, nullptr, mNVGContext, 60.0f, {0.0f, 11.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, true));
		plots.push_back(new Plot ( window, "back ortho", {400, 400}, 4, plot_data, false, nullptr, mNVGContext, 60.0f, {0.0f, 0.0f, -15.0f}, { 0.0f, -M_PI / 2.0f, 0.0f}, true));
		plots.push_back(new Plot ( window, "left ortho", {400, 400}, 5, plot_data, false, nullptr, mNVGContext, 60.0f, { -15.0f, 0.0f, 0.0f}, {0.0f, -M_PI / 4.0f, 0.0f}, true));

		int number_of_cameras = plots.size();
		plot_data->e_vertices.resize(3, 2 * number_of_cameras);
		plot_data->e_colors.resize(3, 2 * number_of_cameras);

		plot_data->e_colors.col(0) << 1, 0, 0; plot_data->e_colors.col(1) << 1, 0, 0;
		plot_data->e_colors.col(2) << 0, 1, 0; plot_data->e_colors.col(3) << 0, 1, 0;
		plot_data->e_colors.col(4) << 0, 0, 1; plot_data->e_colors.col(5) << 0, 0, 1;
		plot_data->e_colors.col(6) << 0.5, 0.5, 0; plot_data->e_colors.col(7) << 0.5, 0.5, 0;
		plot_data->e_colors.col(8) << 0.0, 0.5, 0.5; plot_data->e_colors.col(9) << 0.0, 0.5, 0.5;
		plot_data->e_colors.col(10) << 0.5, 0.0, 0.5; plot_data->e_colors.col(11) << 0.5, 0.0, 0.5;

		// share shader data
		for (int i = 0; i < number_of_cameras; i++) {
			plots[i]->master_pointShader = plots[0]->m_pointShader;
		}

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