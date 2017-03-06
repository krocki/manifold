/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-06 10:14:48
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iomanip>

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/glutil.h>
#include <nanogui/graph.h>
#include <nanogui/console.h>

#include <utils.h>
#include <rand/pcg32.h>

#include "fps.h"
#include "aux.h"

#define POINT_SHADER_NAME "pass"
#define POINT_FRAG_FILE "./src/glsl/surf_point.f.glsl"
#define POINT_VERT_FILE "./src/glsl/surf_point.v.glsl"

#define GRID_SHADER_NAME "pass"
#define GRID_FRAG_FILE "./src/glsl/surf_grid.f.glsl"
#define GRID_VERT_FILE "./src/glsl/surf_grid.v.glsl"

#define DEF_WIDTH 640
#define DEF_HEIGHT 480
#define SCREEN_NAME "Plot"

#define DEF_NUM_POINTS 10000

class SurfacePlot : public nanogui::Screen {

  public:
	SurfacePlot () :
		nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME), m_pointCount(DEF_NUM_POINTS) {

		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		initialize();
	}

	void initialize() {

		/* FPS GRAPH */
		graph_fps = add<nanogui::Graph> ( "" );
		graph_fps->values().resize ( FPS.size() );
		graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

		performLayout(mNVGContext);

		// init shaders
		m_pointShader = new nanogui::GLShader();
		m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );
		m_gridShader = new nanogui::GLShader();
		m_gridShader->initFromFiles ( GRID_SHADER_NAME, GRID_VERT_FILE, GRID_FRAG_FILE );

		drawAll();

		refresh();
		setVisible(true);
		framebufferSizeChanged();

	}

	void generate_points() {

		pcg32 rng;
		positions.resize ( 3, m_pointCount );
		colors.resize ( 3, m_pointCount );

		float yRange = 2.0;
		float yMax = 1.0;

		for ( size_t i = 0; i < m_pointCount; i++ ) {

			float x = rng.nextFloat() * 2.0f - 1.0f;//rand_float(-1.0f, 1.0f);
			float y = rng.nextFloat() * 2.0f - 1.0f;//rand_float(-1.0f, 1.0f);
			float z = hat ( x, y );

			positions.col ( i ) << x, y, z;
			colors.col ( i ) = hslToRgb ( 0.6 * ( yMax - z ) / yRange, 1, 0.5 );

		}

	}

	void refresh() {

		generate_points();

		/* Upload points to GPU */
		m_pointShader->bind();
		m_pointShader->uploadAttrib("position", positions);
		m_pointShader->uploadAttrib("color", colors);

	}

	void framebufferSizeChanged() {


	}

	void drawContents() {

		/* Set up a perspective camera matrix */
		Eigen::Matrix4f view, proj, model;
		view = nanogui::lookAt(Eigen::Vector3f(0, 0, 2), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
		const float viewAngle = 30, near = 0.01, far = 100;
		float fH = std::tan(viewAngle / 360.0f * M_PI) * near;
		float fW = fH * (float) mSize.x() / (float) mSize.y();
		proj = nanogui::frustum(-fW, fW, -fH, fH, near, far);

		model.setIdentity();
		model = nanogui::translate(Eigen::Vector3f(0.0f, 0.0f, -3.0f));

		/* Render the point set */
		Eigen::Matrix4f mvp = proj * view * model;
		m_pointShader->bind();
		m_pointShader->setUniform("mvp", mvp);
		glPointSize(1);
		glEnable(GL_DEPTH_TEST);
		m_pointShader->drawArray(GL_POINTS, 0, m_pointCount);

		// perf stats
		update_FPS(graph_fps);

	}

	void console ( const char *pMsg, ... ) {

		UNUSED(pMsg);
		// char buffer[4096];
		// std::va_list arg;
		// va_start ( arg, pMsg );
		// std::vsnprintf ( buffer, 4096, pMsg, arg );
		// va_end ( arg );
		// log_str.append ( buffer );

	}

	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
			return true;

		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			setVisible ( false );
			return true;
		}

		return false;
	}

	~SurfacePlot() {

		delete m_pointShader;
		delete m_gridShader;

	}

	// shaders
	nanogui::GLShader *m_pointShader = nullptr;
	nanogui::GLShader *m_gridShader = nullptr;

	// GUI widgets
	nanogui::CheckBox *m_gridCheckBox;
	nanogui::Graph *graph_fps;

	// other params
	// # of points, # of grid lines
	size_t m_pointCount, m_lineCount;

	// coords of points
	Eigen::MatrixXf positions;
	Eigen::MatrixXf colors;

	/***************************/


	// void load_shaders() {

	// 	mShader.initFromFiles ( PASSTHROUGH_SHADER_NAME, PASSTHROUGH_VERT_FILE, PASSTHROUGH_FRAG_FILE );

	// 	//* FUNCTION */

	// 	nanogui::MatrixXu indices ( 1, N );
	// 	Eigen::MatrixXf positions ( 3, N );
	// 	Eigen::MatrixXf colors ( 3, N );

	// 	float yRange = 2.0;
	// 	float yMax = 1.0;

	// 	for ( size_t i = 0; i < N; i++ ) {

	// 		indices.col ( i ) << i;

	// 		float x = rand_float ( -1.0, 1.0 );
	// 		float y = rand_float ( -1.0, 1.0 );
	// 		float z = hat ( x, y );

	// 		positions.col ( i ) << x, y, z;
	// 		colors.col ( i ) = hslToRgb ( 0.6 * ( yMax - z ) / yRange, 1, 0.5 );

	// 	}

	// 	// bind the shader and upload vertex positions and indices

	// 	mShader.bind();
	// 	mShader.uploadIndices ( indices );
	// 	mShader.uploadAttrib ( "vertex_in_position", positions );
	// 	mShader.uploadAttrib ( "vertex_in_color", colors );

	// 	// init camera
	// 	position = Eigen::Vector3f::Zero();
	// 	position[2] = -15.0f;

	// 	angle = Eigen::Vector3f::Zero();

	// 	drag_sensitivity = 5.0f;
	// 	scroll_sensitivity = 10.0f;
	// 	keyboard_sensitivity = 0.1f;

	// 	// setup perspective
	// 	float near = 1.0f;
	// 	float far = 100.0f;
	// 	float fov = 10.0f;
	// 	float h = std::tan(fov / 360.0f * M_PI) * near;
	// 	float aspect_ratio = (float)size() [0] / (float)size() [1];
	// 	float w = h * aspect_ratio;

	// 	Eigen::Matrix4f projection_matrix = nanogui::frustum(-w, w, -h, h, near, far);

	// 	mShader.setUniform( "perspective", projection_matrix); 	//perspective matrix
	// 	mShader.setUniform( "offset", position);				//3D position
	// 	mShader.setUniform( "angle", angle); 					//rotation angles

	// }

	// void init() {

	// 	int glfw_window_width, glfw_window_height;
	// 	glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );

	// 	// get GL debug info
	// 	console ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
	// 	console ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
	// 	console ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

	// 	console ( "glfwGetWindowSize(): %d x %d\n", glfw_window_width, glfw_window_height );

	// 	//bottom left graph
	// 	graph_fps = add<nanogui::Graph> ( "" );
	// 	graph_fps->values().resize ( FPS.size() );
	// 	graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
	// 	graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

	// 	/* console */
	// 	show_console = false;

	// 	console_window = new nanogui::Window ( this, "" );
	// 	console_window->setVisible ( show_console );

	// 	// console_window->setLayout ( new GroupLayout ( 5, 5, 0, 0 ) );
	// 	console_panel = new nanogui::Console ( console_window );
	// 	console_panel->setFontSize ( 12 );

	// 	// debug footer message
	// 	footer_message = new nanogui::Console ( this );
	// 	footer_message->setFontSize ( 12 );
	// 	footer_message_string = "";

	// 	resizeEvent ( { glfw_window_width, glfw_window_height } );

	// }

	// virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {
	// 	if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
	// 		return true;

	// 	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
	// 		setVisible ( false );
	// 		return true;
	// 	}

	// 	if ( key == GLFW_KEY_TAB && action == GLFW_PRESS ) {

	// 		show_console = !show_console;
	// 		console_window->setVisible ( show_console );

	// 	}

	// 	// execute on press or hold, skip on release
	// 	/*	moved to process_keyboard()
	// 		if ( action == GLFW_PRESS || action == GLFW_REPEAT ) {

	// 	}*/

	// 	return false;
	// }

	// // smooth keys - TODO: move to nanogui
	// void process_keyboard() {

	// 	// keyboard management
	// 	/*	TODO: move to nanogui - need to modify keyboardEvent to allow smooth opeartion */
	// 	// translation
	// 	if ( glfwGetKey ( glfwWindow(), 'A' ) == GLFW_PRESS ) position[0] -= 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'D' ) == GLFW_PRESS ) position[0] += 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'S' ) == GLFW_PRESS ) position[1] -= 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'W' ) == GLFW_PRESS ) position[1] += 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'Z' ) == GLFW_PRESS ) position[2] -= 0.5 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'C' ) == GLFW_PRESS ) position[2] += 0.5 * keyboard_sensitivity;

	// 	// rotation around x, y, z axes
	// 	if ( glfwGetKey ( glfwWindow(), 'Q' ) == GLFW_PRESS ) angle[0] -= 0.025 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), 'E' ) == GLFW_PRESS ) angle[0] += 0.025 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), GLFW_KEY_UP ) == GLFW_PRESS ) angle[1] -= 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), GLFW_KEY_DOWN ) == GLFW_PRESS ) angle[1] += 0.05 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), GLFW_KEY_LEFT ) == GLFW_PRESS ) angle[2] -= 0.1 * keyboard_sensitivity;
	// 	if ( glfwGetKey ( glfwWindow(), GLFW_KEY_RIGHT ) == GLFW_PRESS ) angle[2] += 0.1 * keyboard_sensitivity;

	// }

	// virtual bool mouseDragEvent ( const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button,
	//                               int modifiers ) {


	// 	UNUSED(p, button, modifiers);

	// 	angle[0] += ( float ) rel[0] / ( ( float ) size() [0] / drag_sensitivity );
	// 	angle[1] += ( float ) rel[1] / ( ( float ) size() [1] / drag_sensitivity );

	// 	return true;

	// }

	// virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel) {

	// 	UNUSED(p);

	// 	angle[1] -=  ( float ) rel[1] / ( ( float ) size() [1] / scroll_sensitivity );
	// 	angle[2] -=  ( float ) rel[0] / ( ( float ) size() [0] / scroll_sensitivity );

	// 	return true;
	// }

	// virtual void draw ( NVGcontext *ctx ) {

	// 	process_keyboard();
	// 	graph_fps->values() = Eigen::Map<Eigen::VectorXf> ( FPS.data(), FPS.size() );

	// 	char str[16];
	// 	int last_avg = 10;

	// 	sprintf ( str, "%3.1f FPS\n", graph_fps->values().block ( FPS.size() - 1 - last_avg, 0, last_avg, 1 ).mean() );

	// 	graph_fps->setHeader ( str );
	// 	console_panel->setValue ( log_str );

	// 	// debug
	// 	std::stringstream ss;
	// 	ss << std::setprecision ( 2 ) << "pos = (" << std::setw ( 5 ) << position[0] << ", " <<
	// 	   std::setw ( 5 ) << position[1] << ", " << std::setw ( 5 ) << position[2] <<
	// 	   ")" << ", ang: (" << angle[0] << ", " << std::setw ( 5 ) << angle[1] << ", " <<
	// 	   std::setw ( 5 ) << angle[2] << ")" << "\nres = " << size() [0] << "x" << size() [1] <<
	// 	   '\n' << "drag: " << mDragActive << '\n';

	// 	footer_message_string = ss.str();
	// 	footer_message->setValue ( footer_message_string );

	// 	/* Draw the user interface */
	// 	Screen::draw ( ctx );
	// 	update_FPS();

	// }

	// virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

	// 	// FPS graph
	// 	int graph_width = 110;
	// 	int graph_height = 15;
	// 	graph_fps->setPosition ( {5, size[1] - graph_height - 5} );
	// 	graph_fps->setSize ( {graph_width, graph_height } );

	// 	// console
	// 	int console_width = 250;
	// 	int console_height = size[1] - 10;
	// 	console_window->setPosition ( {size[0] - console_width - 5, 5} );
	// 	console_window->setSize ( {console_width, console_height} );
	// 	console_panel->setPosition ( {5, 5} );
	// 	console_panel->setWidth ( console_width - 10 );
	// 	console_panel->setHeight ( console_height - 10 );

	// 	// footer
	// 	int footer_height = 40;
	// 	int footer_width = 550;
	// 	footer_message->setPosition ( {5, 5} );
	// 	footer_message->setSize ( {footer_width, footer_height} );

	// 	performLayout();

	// 	return true;

	// }

	// virtual void drawContents() {


	// 	mShader.bind();

	// 	glEnable ( GL_DEPTH_TEST );

	// 	mShader.setUniform( "offset", position);
	// 	mShader.setUniform( "angle", angle);

	// 	mShader.drawIndexed ( GL_POINTS, 0, N );
	// 	glDisable ( GL_DEPTH_TEST );
	// }

	// nanogui::GLShader mShader;
	// nanogui::Graph *graph_fps;
	// nanogui::Window *console_window;
	// nanogui::Console *console_panel, *footer_message;

	// //for manipulating view
	// Eigen::Vector3f position, angle;

	// std::string log_str, footer_message_string;

	// bool show_console;
	// float drag_sensitivity, scroll_sensitivity, keyboard_sensitivity;

};

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		nanogui::init();

		SurfacePlot* screen = new SurfacePlot();
		nanogui::mainloop ( 1 );

		delete screen;
		nanogui::shutdown();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;

}
