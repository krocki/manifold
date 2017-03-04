/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 16:20:38
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 16:20:42
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