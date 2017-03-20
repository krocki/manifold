/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-19 20:13:22
*/

#include <thread>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/manifoldscreen.h"

int compute() {

	return 0;
}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		nanogui::Screen *screen = new GUI();

		// launch a compute thread
		std::thread compute_thread ( compute );

		nanogui::mainloop ( 1 );

		delete screen;
		nanogui::shutdown();
		compute_thread.join();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}