/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-06 11:35:00
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
#include <nanogui/slider.h>
#include <nanogui/console.h>
#include <nanogui/textbox.h>
#include <nanogui/checkbox.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>

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

		m_window = new nanogui::Window(this, "");
		m_window->setPosition(Eigen::Vector2i(5, 55));
		m_window->setLayout(new nanogui::GroupLayout());

		nanogui::Label *l = new nanogui::Label(m_window, "# of points", "sans-bold");
		l->setFontSize ( 10 );

		/* Add a slider and set defaults */
		m_pointCountSlider = new nanogui::Slider(m_window);
		m_pointCountSlider->setFixedWidth(80);
		m_pointCountSlider->setCallback([&](float) { refresh(); });

		m_pointCountSlider->setValue(7.f / 15.f);

		/* Add a textbox and set defaults */
		m_pointCountBox = new nanogui::TextBox(m_window);
		m_pointCountBox->setFixedSize(Eigen::Vector2i(80, 25));
		m_pointCountBox->setFontSize ( 10 );

		m_gridCheckBox = new nanogui::CheckBox(m_window, "Grid");
		m_gridCheckBox->setCallback([&](bool) { refresh(); });

		/* FPS GRAPH */
		graph_fps = add<nanogui::Graph> ( "" );
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

		m_pointCount = (int) std::pow(2.f, 15 * m_pointCountSlider->value() + 5);

		generate_points();

		/* Upload points to GPU */
		m_pointShader->bind();
		m_pointShader->uploadAttrib("position", positions);
		m_pointShader->uploadAttrib("color", colors);

		/* Upload lines to GPU */
		if (m_gridCheckBox->checked()) {

			// draw grid
			m_lineCount = 4 * 2;
			line_positions.resize(3, m_lineCount);

			//L
			line_positions.col(0) << -1, -1, 0; line_positions.col(1) << -1, 1, 0;
			//R
			line_positions.col(2) << 1, -1, 0; line_positions.col(3) << 1, 1, 0;
			//B
			line_positions.col(4) << -1, -1, 0; line_positions.col(5) << 1, -1, 0;
			//T
			line_positions.col(6) << -1, 1, 0; line_positions.col(7) << 1, 1, 0;

			m_gridShader->bind();
			m_gridShader->uploadAttrib("position", line_positions);
		}

		/* Update user interface */
		std::string str;

		if (m_pointCount > 1000000) {
			m_pointCountBox->setUnits("M");
			str = std::to_string(m_pointCount * 1e-6f);
		} else if (m_pointCount > 1000) {
			m_pointCountBox->setUnits("K");
			str = std::to_string(m_pointCount * 1e-3f);
		} else {
			m_pointCountBox->setUnits(" ");
			str = std::to_string(m_pointCount);
		}

		m_pointCountBox->setValue(str);
		m_pointCountSlider->setValue((std::log((float) m_pointCount) / std::log(2.f) - 5) / 15);
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

		bool drawGrid = m_gridCheckBox->checked();
		if (drawGrid) {
			m_gridShader->bind();
			m_gridShader->setUniform("mvp", mvp);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_gridShader->drawArray(GL_LINES, 0, m_lineCount);
			glDisable(GL_BLEND);
		}

		// perf stats
		update_FPS(graph_fps);

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
	nanogui::Window *m_window;
	nanogui::CheckBox *m_gridCheckBox;
	nanogui::Graph *graph_fps;
	nanogui::Slider *m_pointCountSlider;
	nanogui::TextBox *m_pointCountBox;

	// other params
	// # of points, # of grid lines
	size_t m_pointCount, m_lineCount;

	// coords of points
	Eigen::MatrixXf positions;
	Eigen::MatrixXf line_positions;
	Eigen::MatrixXf colors;

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
