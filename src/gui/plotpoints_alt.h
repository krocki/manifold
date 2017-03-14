
#ifndef __PLOTPOINTS_H__
#define __PLOTPOINTS_H__

#include <nanogui/glcanvas.h>
#include <nanogui/checkbox.h>
#define POINT_SHADER_NAME "point_shader"
#define POINT_FRAG_FILE "./src/glsl/surf_point.f.glsl"
#define POINT_VERT_FILE "./src/glsl/surf_point.v.glsl"

#define GRID_SHADER_NAME "grid_shader"
#define GRID_FRAG_FILE "./src/glsl/surf_grid.f.glsl"
#define GRID_VERT_FILE "./src/glsl/surf_grid.v.glsl"

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/layout.h>
#include <nanogui/imagepanel.h>

// FPS
#include "fps.h"

class SurfWindow : public nanogui::Window {

	public:
	
		SurfWindow ( Widget *parent, const std::string &title ) : nanogui::Window ( parent, title ) {
		
			/* FPS GRAPH */
			m_window = new nanogui::Window ( this, "" );
			m_window->setLayout ( new nanogui::GroupLayout() );
			
			//legend
			nanogui::Graph *graph = new nanogui::Graph ( m_window, "", nanogui::GraphType::GRAPH_LEGEND );
			graph->values().resize ( 10 );
			graph->values() << 1, 1, 1, 1, 1, 1, 1, 1, 1, 1;
			
			m_grid = new nanogui::CheckBox ( this, "Grid" );
			m_polar = new nanogui::CheckBox ( this, "Polar" );
			
			m_grid->setChecked ( true );
			
		}
		
		nanogui::Window *m_window;
		nanogui::CheckBox *m_grid, *m_polar;
		
};

class SurfPlot : public nanogui::GLCanvas {
	public:
		SurfPlot ( Widget *parent, const Eigen::Vector2i &w_size,
				   SurfWindow &helper_window ) : widgets ( helper_window ) , nanogui::GLCanvas ( parent ) {
			using namespace nanogui;
			
			
			setSize ( w_size );
			
			setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			setDrawBorder ( true );
			
			// init shaders
			m_pointShader = new nanogui::GLShader();
			m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );
			m_gridShader = new nanogui::GLShader();
			m_gridShader->initFromFiles ( GRID_SHADER_NAME, GRID_VERT_FILE, GRID_FRAG_FILE );
			
			refresh();
			setVisible ( true );
			
			// set default view
			Eigen::Matrix3f mat;
			
			mat ( 0, 0 ) = 0.813283; mat ( 0, 1 ) =  -0.424654; mat ( 0, 2 ) = -0.397795;
			mat ( 1, 0 ) = -0.0271025; mat ( 1, 1 ) =  -0.710554; mat ( 1, 2 ) = 0.70312;
			mat ( 2, 0 ) = -0.581237; mat ( 2, 1 ) =  -0.561054; mat ( 2, 2 ) = -0.589391;
			
			Eigen::Quaternionf q ( mat );
			m_arcball.setState ( q );
			setSize ( w_size );
			m_arcball.setSize ( w_size );
			
			translation = Eigen::Vector3f::Zero();
			model_angle = Eigen::Vector3f::Zero();
			
			drag_sensitivity = 5.0f;
			scroll_sensitivity = 10.0f;
			keyboard_sensitivity = 0.1f;
			
		}
		
		void generate_points() {
		
			m_pointCount = nn->codes.cols();
			
			positions.resize ( 3, m_pointCount );
			colors.resize ( 3, m_pointCount );
			
			for ( size_t i = 0; i < m_pointCount; i++ ) {
			
				Eigen::VectorXf coords;
				int label;
				
				coords = nn->codes.col ( i );
				label = nn->codes_idxs ( i );
				
				float x, y, z;
				
				//if (spherical) {
				
				// float phi = coords[2];
				
				// 3d
				// x =	r * cos ( theta ) * sin ( phi );
				// y = r * sin ( theta )  * cos ( phi );
				// z = r * cos ( phi );
				
				//}
				
				if ( widgets.m_polar->checked() ) {
				
					float r = coords[0];
					float theta = coords[1];
					x =	r * cos ( theta );
					y = r * sin ( theta );
				}
				else {
					x = coords[0];
					y = coords[1];
					
				}
				if ( coords.size() > 2 ) z = coords[2];
				else z = 0.0f;
				
				positions.col ( i ) << x / 30.0f, y / 30.0f, z / 30.0f;
				// std::cout << positions.col ( i ) << std::endl;
				nanogui::Color c = nanogui::parula_lut[label];
				colors.col ( i ) << c.r(), c.g(), c.b();
				
			}
			
		}
		
		bool mouseMotionEvent ( const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers ) override {
			// if ( !SurfPlot::mouseMotionEvent ( p, rel, button, modifiers ) )
			m_arcball.motion ( p );
			// std::cout << m_arcball.state().matrix() << std::endl;
			return true;
		}
		
		bool mouseButtonEvent ( const Eigen::Vector2i &p, int button, bool down, int modifiers ) override {
		
			// if ( !SurfPlot::mouseButtonEvent ( p, button, down, modifiers ) ) {
			if ( button == GLFW_MOUSE_BUTTON_1 )
				m_arcball.button ( p, down );
			// }
			
			return true;
		}
		
		virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) override {
		
			// if ( SurfPlot::keyboardEvent ( key, scancode, action, modifiers ) )
			// 	return true;
			
			if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
				setVisible ( false );
				return true;
			}
			
			return false;
		}
		
		void refresh() {
		
			m_pointCount = ( int ) m_pointCount;
			
			generate_points();
			
			/* Upload points to GPU */
			m_pointShader->bind();
			m_pointShader->uploadAttrib ( "position", positions );
			m_pointShader->uploadAttrib ( "color", colors );
			
			/* Upload lines to GPU */
			if ( widgets.m_grid->checked() ) {
			
				// draw grid
				m_lineCount = 12 * 2;
				line_positions.resize ( 3, m_lineCount );
				
				line_positions.col ( 0 ) << 0, 0, 0; line_positions.col ( 1 ) << 0, 1, 0; //L
				line_positions.col ( 2 ) << 1, 0, 0; line_positions.col ( 3 ) << 1, 1, 0; //R
				line_positions.col ( 4 ) << 0, 0, 0; line_positions.col ( 5 ) << 1, 0, 0; //B
				line_positions.col ( 6 ) << 0, 1, 0; line_positions.col ( 7 ) << 1, 1, 0; //T
				
				line_positions.col ( 8 ) << 0, 0, 0; line_positions.col ( 9 ) << 0, 0, 1;
				line_positions.col ( 10 ) << 1, 0, 0; line_positions.col ( 11 ) << 1, 0, 1;
				line_positions.col ( 12 ) << 0, 1, 0; line_positions.col ( 13 ) << 0, 1, 1;
				line_positions.col ( 14 ) << 1, 1, 0; line_positions.col ( 15 ) << 1, 1, 1;
				
				line_positions.col ( 16 ) << 0, 0, 1; line_positions.col ( 17 ) << 0, 1, 1; //UL
				line_positions.col ( 18 ) << 1, 0, 1; line_positions.col ( 19 ) << 1, 1, 1; //UR
				line_positions.col ( 20 ) << 0, 0, 1; line_positions.col ( 21 ) << 1, 0, 1; //UB
				line_positions.col ( 22 ) << 0, 1, 1; line_positions.col ( 23 ) << 1, 1, 1; //UT
				
				m_gridShader->bind();
				m_gridShader->uploadAttrib ( "position", line_positions );
			}
			
		}
		
		// smooth keys - TODO: move to nanogui
		void process_keyboard() {
		
			// keyboard management
			/*	TODO: move to nanogui - need to modify keyboardEvent to allow smooth opeartion */
			// translation
			if ( glfwGetKey ( screen->glfwWindow(), 'A' ) == GLFW_PRESS ) translation[0] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'D' ) == GLFW_PRESS ) translation[0] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'S' ) == GLFW_PRESS ) translation[1] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'W' ) == GLFW_PRESS ) translation[1] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'Z' ) == GLFW_PRESS ) translation[2] -= 0.5 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'C' ) == GLFW_PRESS ) translation[2] += 0.5 * keyboard_sensitivity;
			
			if ( glfwGetKey ( screen->glfwWindow(), '1' ) == GLFW_PRESS ) fov += 0.05;
			if ( glfwGetKey ( screen->glfwWindow(), '2' ) == GLFW_PRESS ) fov -= 0.05;
			
			// rotation around x, y, z axes
			if ( glfwGetKey ( screen->glfwWindow(), 'Q' ) == GLFW_PRESS ) model_angle[0] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'E' ) == GLFW_PRESS ) model_angle[0] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_UP ) == GLFW_PRESS ) model_angle[1] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_DOWN ) == GLFW_PRESS ) model_angle[1] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_LEFT ) == GLFW_PRESS ) model_angle[2] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_RIGHT ) == GLFW_PRESS ) model_angle[2] += 0.05 * keyboard_sensitivity;
			
			// std::cout << "translation: " << translation << std::endl;
			// std::cout << "model_angle: " << model_angle << std::endl;
			
		}
		
		void drawGL() override {
		
			process_keyboard();
			
			/* Set up a perspective camera matrix */
			Eigen::Matrix4f view, proj, model;
			
			view = nanogui::lookAt ( Eigen::Vector3f ( 0, 0, 1 ), Eigen::Vector3f ( 0, 0, 0 ), Eigen::Vector3f ( 0, 1, 0 ) );
			const float near = 0.01, far = 100;
			float fH = std::tan ( fov / 360.0f * M_PI ) * near;
			float fW = fH * ( float ) mSize.x() / ( float ) mSize.y();
			proj = nanogui::frustum ( -fW, fW, -fH, fH, near, far );
			
			model.setIdentity();
			model = m_arcball.matrix() * model;
			
			// Eigen::Quaternionf q = Eigen::AngleAxisf ( model_angle[0] * M_PI, Eigen::Vector3f::UnitX() )
			// 					   * Eigen::AngleAxisf ( model_angle[1] * M_PI,  Eigen::Vector3f::UnitY() )
			// 					   * Eigen::AngleAxisf ( model_angle[2] * M_PI, Eigen::Vector3f::UnitZ() );
			
			
			// model.block ( 0, 0, 3, 3 ) = q.matrix() * model.block ( 0, 0, 3, 3 );
			model = nanogui::translate ( Eigen::Vector3f ( translation ) ) * model;
			
			/* Render the point set */
			Eigen::Matrix4f mvp = proj * view * model;
			m_pointShader->bind();
			m_pointShader->setUniform ( "mvp", mvp );
			glPointSize ( 1 );
			glEnable ( GL_DEPTH_TEST );
			m_pointShader->drawArray ( GL_POINTS, 0, m_pointCount );
			
			if ( widgets.m_grid->checked() ) {
				m_gridShader->bind();
				m_gridShader->setUniform ( "mvp", mvp );
				glEnable ( GL_BLEND );
				glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				m_gridShader->drawArray ( GL_LINES, 0, m_lineCount );
				glDisable ( GL_BLEND );
			}
			
			// perf stats
			update_FPS ( *graph_data );
			
		}
		
		~SurfPlot() {
		
			delete m_pointShader;
			delete m_gridShader;
			
		}
		
		size_t m_pointCount = 1;
		size_t m_lineCount = 4 * 2;
		
		// shaders
		nanogui::GLShader *m_pointShader = nullptr;
		nanogui::GLShader *m_gridShader = nullptr;
		nanogui::Arcball m_arcball;
		
		Eigen::VectorXf *graph_data;
		
		Eigen::Vector3f translation;
		Eigen::Vector3f model_angle;
		
		// coords of points
		Eigen::MatrixXf positions;
		Eigen::MatrixXf line_positions;
		Eigen::MatrixXf colors;
		
		SurfWindow &widgets;
		
		float fov = 60;
		float drag_sensitivity, scroll_sensitivity, keyboard_sensitivity;
		
};

#endif
