/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-11 20:59:04
*/

/* nbody */

#include <thread>
#include <unistd.h>

#include <iostream>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/glutil.h>
#include <nanogui/graph.h>
#include <nanogui/slider.h>
#include <nanogui/console.h>
#include <nanogui/textbox.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>

//helpers
#include <utils.h>
#include "fps.h"
#include "aux.h"

#define POINT_SHADER_NAME "point_shader"
#define POINT_FRAG_FILE "./src/glsl/surf_point.f.glsl"
#define POINT_VERT_FILE "./src/glsl/surf_point.v.glsl"

#define DEF_WIDTH 1024
#define DEF_HEIGHT 768
#define SCREEN_NAME "nbody"

// 10^
#define DEF_NUM_POINTS 5.0f // 100k
#define MIN_NUM_POINTS 1.0f // 10
#define MAX_NUM_POINTS 7.0f // 10M

size_t g_pointCount;
Eigen::MatrixXf g_positions;
Eigen::MatrixXf g_colors;

bool changed;

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
		m_window = new nanogui::Window(this, "");
		m_window->setPosition(Eigen::Vector2i(5, 55));
		m_window->setLayout(new nanogui::GroupLayout());

		nanogui::Label *l = new nanogui::Label(m_window, "# of points", "sans-bold");
		l->setFontSize ( 10 );

		/* Add a slider and set defaults */
		g_pointCountSlider = new nanogui::Slider(m_window);
		g_pointCountSlider->setFixedWidth(80);
		g_pointCountSlider->setRange(std::pair<float, float>(MIN_NUM_POINTS, MAX_NUM_POINTS));
		g_pointCountSlider->setCallback([&](float) { refresh(); });

		g_pointCountSlider->setValue(DEF_NUM_POINTS);

		/* Add a textbox and set defaults */
		g_pointCountBox = new nanogui::TextBox(m_window);
		g_pointCountBox->setFixedSize(Eigen::Vector2i(80, 25));
		g_pointCountBox->setFontSize ( 10 );

		/* FPS GRAPH */
		graph_fps = add<nanogui::Graph> ( "" );
		graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );

		performLayout(mNVGContext);

		// init shaders
		m_pointShader = new nanogui::GLShader();
		m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		/* * * * * * * * * * * */

		drawAll();
		setVisible(true);
		framebufferSizeChanged();

		glfwSwapInterval(vsync);

		performLayout();
		resizeEvent ( { glfw_window_width, glfw_window_height } );

	}

	void refresh() {

		// g_pointCount = (int) std::pow(10.0f, g_pointCountSlider->value());

		/* Upload points to GPU */
		m_pointShader->bind();
		m_pointShader->uploadAttrib("position", g_positions);
		m_pointShader->uploadAttrib("color", g_colors);

		/* Update user interface */
		// std::string str;

		// if (g_pointCount > 1000000) {
		// 	g_pointCountBox->setUnits("M");
		// 	str = std::to_string(g_pointCount * 1e-6f);
		// } else if (g_pointCount > 1000) {
		// 	g_pointCountBox->setUnits("K");
		// 	str = std::to_string(g_pointCount * 1e-3f);
		// } else {
		// 	g_pointCountBox->setUnits(" ");
		// 	str = std::to_string(g_pointCount);
		// }

		// g_pointCountBox->setValue(str);
		// g_pointCountSlider->setValue(std::log((float) g_pointCount) / std::log(10.0f));
	}

	void framebufferSizeChanged() { m_arcball.setSize(mSize); }


	void drawContents() {

		if (changed) {

			refresh();
			changed = false;
		}

		/* Set up a perspective camera matrix */
		Eigen::Matrix4f view, proj, model;
		view = nanogui::lookAt(Eigen::Vector3f(0, 0, 2), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));

		const float viewAngle = 30, near = 0.01, far = 100;
		float fH = std::tan(viewAngle / 360.0f * M_PI) * near;
		float fW = fH * (float) mSize.x() / (float) mSize.y();

		proj = nanogui::frustum(-fW, fW, -fH, fH, near, far);

		model.setIdentity();
		model = m_arcball.matrix() * model;
		model = nanogui::translate(Eigen::Vector3f(0.0f, 0.0f, -3.0f)) * model;


		/* Render the point set */
		Eigen::Matrix4f mvp = proj * view * model;
		m_pointShader->bind();
		m_pointShader->setUniform("mvp", mvp);
		glPointSize(1);
		glEnable(GL_DEPTH_TEST);
		m_pointShader->drawArray(GL_POINTS, 0, g_pointCount);

		// perf stats
		update_FPS(graph_fps);

	}

	/* event handlers */

	bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) {
		if (!Screen::mouseMotionEvent(p, rel, button, modifiers))
			m_arcball.motion(p);
		return true;
	}

	bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) {

		if (!Screen::mouseButtonEvent(p, button, down, modifiers)) {
			if (button == GLFW_MOUSE_BUTTON_1)
				m_arcball.button(p, down);
		}

		return true;
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

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		UNUSED(size);
		performLayout();
		return true;

	}

	~GUI() { /* free resources */

		delete m_pointShader;

	}

  protected:

	// shaders
	nanogui::GLShader *m_pointShader = nullptr;

	// GUI widgets
	nanogui::Window *m_window;
	nanogui::Graph *graph_fps;
	nanogui::Slider *g_pointCountSlider;
	nanogui::TextBox *g_pointCountBox;

	nanogui::Arcball m_arcball;

	int glfw_window_width, glfw_window_height;
	bool vsync;

};

GUI* screen;

void generate_points(Eigen::MatrixXf &positions, Eigen::MatrixXf &colors, size_t N) {

	g_pointCount = N;

	positions.resize ( 3, g_pointCount );
	colors.resize ( 3, g_pointCount );

	std::cout << g_pointCount << std::endl;

	for ( size_t i = 0; i < g_pointCount; i++ ) {

		float x = rand_float( -0.5f, 0.5f );
		float y = rand_float( -0.5f, 0.5f );
		float z = rand_float( -0.0f, 0.0f );

		positions.col ( i ) << x, y, z;
		colors.col ( i ) = Eigen::Vector3f(x, y, z);

	}

	changed = true;
}

int compute() {

	/* work until main window is open */
	while (screen->getVisible()) {

		for (size_t i = 0; i < g_pointCount; i++) {

			Eigen::VectorXf t = g_positions.col ( i );

			float x = 0;
			float y = 0;
			float z = rand_float( -0.001f, 0.001f );;

			t << t.x() + x, t.y() + y, t.z() + z;

			g_positions.col ( i ) = t;
			g_colors.col ( i ) = g_positions.col ( i );

		}

		changed = true;
		usleep(1000);
	}

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		generate_points(g_positions, g_colors, (size_t) std::pow(10.0f, DEF_NUM_POINTS));

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

		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;

}
