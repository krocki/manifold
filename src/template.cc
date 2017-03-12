/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-11 20:18:24
*/

/* A template for GUI + compute thread */

#include <thread>
#include <unistd.h>

#include <nanogui/glutil.h>
#include <nanogui/screen.h>

//helpers
#include <utils.h>

#define DEF_WIDTH 600
#define DEF_HEIGHT 400
#define SCREEN_NAME "SCREEN_NAME"

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

		/* * * * * * * * * * * */

		drawAll();
		setVisible(true);
		framebufferSizeChanged();

		glfwSwapInterval(vsync);

		performLayout();
		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}

	void drawContents() { }

	/* event handlers */

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		/* process subcomponents */
		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		/* close */
		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
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

};

GUI* screen;

int compute() {

	/* work until main window is open */
	while (screen->getVisible()) {

		printf("compute()\n");
		usleep(100000);
	}

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		screen = new GUI();
		// remember that compute thread cannot start until this is done
		// change the order if needed

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
