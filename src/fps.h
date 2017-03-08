/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 16:20:38
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-07 11:27:53
*/

// FPS
#define FPS_HISTORY_SIZE 50

double last_time = glfwGetTime();
int num_frames = 0;

std::vector<float> FPS ( FPS_HISTORY_SIZE, 0 );

void update_FPS ( void ) {

	double current_time = glfwGetTime();
	num_frames++;

	double t = current_time - last_time;

	float interval = 0.25f;

	if ( t >= interval ) {

		FPS.push_back ( ( num_frames / t ) );
		FPS.erase ( FPS.begin() );

		num_frames = 0;
		last_time += interval;

	}
}

void update_FPS ( nanogui::Graph* g ) {

	if (g->values().rows() == 0) {

		g->values() = Eigen::VectorXf(FPS_HISTORY_SIZE);
		g->values().setZero();

	}

	double current_time = glfwGetTime();
	num_frames++;

	double t = current_time - last_time;

	float interval = 0.25f;

	if ( t >= interval ) {

		// shift left and update
		g->values().head(g->values().size() - 1) = g->values().tail(g->values().size() - 1);
		g->values().tail(1)(0) = num_frames / t;

		num_frames = 0;
		last_time += interval;

		// set header
		char str[256];
		int last_avg = 10;

		sprintf ( str, "%3.1f FPS\n", g->values().tail(last_avg).mean() );

		g->setHeader ( str );

	}
}

void update_graph ( nanogui::Graph* g, std::vector<float>& d, float scale = 1.0f, const char* units = "" ) {

	g->values() = Eigen::Map<Eigen::VectorXf> ( d.data(), d.size() );
	g->values() *= scale;

	// set header
	char str[256];
	int last_avg = 10;

	sprintf ( str, "%3.1f %s\n", g->values().tail(last_avg).mean(), units );

	g->setHeader ( str );

}

#define HISTORY_SIZE 1000

void update_graph ( nanogui::Graph* g, float new_val) {

	if (g->values().rows() < HISTORY_SIZE) {

		g->values() = Eigen::VectorXf(HISTORY_SIZE);
		g->values().setZero();

	}

	// shift left and update
	g->values().head(g->values().size() - 1) = g->values().tail(g->values().size() - 1);
	g->values().tail(1)(0) = new_val;

	// set header
	char str[256];
	int last_avg = 10;

	sprintf ( str, "%3.1f\n", g->values().tail(last_avg).mean() );

	g->setHeader ( str );

}
