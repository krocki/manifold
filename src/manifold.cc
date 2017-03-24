/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-23 16:32:04
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/manifoldscreen.h"
#include <compute/functions.h>
GUI *screen;

int compute() {

	PlotData* gl_data = screen->plot_data;
	gl_data->updated();

	/* work until main window is open */
	while (screen->getVisible()) {

		usleep(1000);
		//gl_data->updated();

	}

	return 0;

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

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}