/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-23 14:34:53
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/fplotscreen.h"
#include <compute/functions.h>
GUI *screen;


int compute() {

	size_t point_count = 50000;

	Eigen::MatrixXf velocities;

	PlotData* gl_data = screen->plot_data;
	generate(func3::hat, func1::uniform, gl_data->p_vertices, point_count, 5);
	set({0.0f, 1.0f, 0.0f}, gl_data->p_colors, point_count);

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