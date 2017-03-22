/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-21 22:16:59
*/

#include <thread>
#include <unistd.h>

//nanogui
#include <nanogui/screen.h>

//GUI
#include "gui/manifoldscreen.h"
#include "compute/nbody.h"

GUI *screen;

int compute() {

	size_t point_count = 3000;

	Eigen::MatrixXf velocities;

	PlotData* gl_data = screen->plot_data;
	generate_randn_points(gl_data->p_vertices, point_count);
	generate_points(gl_data->p_colors, point_count, Eigen::Vector3f (0.0f, 1.0f, 0.0f));
	generate_rand_points(velocities, point_count);

	gl_data->updated();

	/* work until main window is open */
	while (screen->getVisible()) {

		nbody::calculate_forces(gl_data->p_vertices, velocities);
		gl_data->updated();
		usleep(10000);
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