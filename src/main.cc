/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 16:21:18
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iomanip>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/theme.h>
#include <nanogui/graph.h>
#include <nanogui/console.h>

#include <utils.h>

#include "fps.h"

#define SCREEN_DEFAULT_WIDTH 640
#define SCREEN_DEFAULT_HEIGHT 480
#define SCREEN_NAME "VAE"

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

		//bottom left graph
		graph_fps = add<nanogui::Graph> ( "" );
		graph_fps->values().resize ( FPS.size() );
		graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

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

		char str[16];
		int last_avg = 10;

		sprintf ( str, "%3.1f FPS\n", graph_fps->values().block ( FPS.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );

		graph_fps->setHeader ( str );
		console_panel->setValue ( log_str );

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

		// FPS graph
		int graph_width = 110;
		int graph_height = 15;
		graph_fps->setPosition ( {5, size[1] - graph_height - 5} );
		graph_fps->setSize ( {graph_width, graph_height } );

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

	nanogui::Graph *graph_fps;
	nanogui::Window *console_window;
	nanogui::Console *console_panel, *footer_message;

	std::string log_str, footer_message_string;

	bool show_console;
};

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		nanogui::init();

		/* scoped variables */ {
			nanogui::ref<Manifold> app = new Manifold();
			app->drawAll();
			app->setVisible ( true );
			nanogui::mainloop ( 1 );
		}

		nanogui::shutdown();
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
