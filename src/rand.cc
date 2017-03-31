/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-30 20:34:10
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/fplotscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

int compute() {

	size_t point_count = 100000;

	PlotData *gl_data = screen->plot_data;

	generate ( std::uniform_real_distribution<> ( -10, 10 ),
	           std::uniform_real_distribution<> ( -10, 10 ),
	           std::uniform_real_distribution<> ( -10, 10 ),
	           gl_data->p_vertices, point_count );

	set ( {0.0f, 0.2f, 0.0f}, gl_data->p_colors, point_count );

	gl_data->updated();

	/* work until main window is open */
	while ( screen->getVisible() )

		usleep ( 1000 );


	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		screen = std::shared_ptr<GUI> ( new GUI() );

		// launch a compute thread
		std::thread compute_thread ( compute );

		nanogui::mainloop ( 1 );

		compute_thread.join();

		nanogui::shutdown();


	}

	catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}