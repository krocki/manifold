
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

#define BOX_SHADER_NAME "box_shader"
#define BOX_FRAG_FILE "./src/glsl/surf_box.f.glsl"
#define BOX_VERT_FILE "./src/glsl/surf_box.v.glsl"

#define MAGBOX_SHADER_NAME "mag_box_shader"
#define MAGBOX_FRAG_FILE "./src/glsl/mag_box.f.glsl"
#define MAGBOX_VERT_FILE "./src/glsl/mag_box.v.glsl"

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/console.h>
#include <nanogui/layout.h>
#include <nanogui/imagepanel.h>
#include <nanogui/label.h>

// FPS
#include "fps.h"

#include "shapes.h"

class SurfWindow : public nanogui::Window {

	public:
	
		SurfWindow ( Widget *parent, const std::string &title ) : nanogui::Window ( parent, title ) {
		
			nanogui::Window *div = new nanogui::Window ( this, "" );
			div->setLayout ( new nanogui::VGroupLayout() );
			
			nanogui::Window *boxes = new nanogui::Window ( div, "" );
			boxes->setLayout ( new nanogui::GroupLayout() );
			m_grid = new nanogui::CheckBox ( boxes, "Grid" );
			m_polar = new nanogui::CheckBox ( boxes, "Polar" );
			m_magbox = new nanogui::CheckBox ( boxes, "Box" );
			show_inputs = new nanogui::CheckBox ( boxes, "Inputs" );
			
			m_grid->setChecked ( true );
			m_polar->setChecked ( true );
			m_magbox->setChecked ( true );
			show_inputs->setChecked ( false );
			magmove = true;
			magboxstate = new nanogui::Label ( boxes, string_format ( "Mag box locked: %d", !magmove ) );
			
			m_window = new nanogui::Window ( this, "" );
			m_window->setLayout ( new nanogui::VGroupLayout ( 15, 15, 15, 0 ) );
			
			nanogui::Window *console_window = new nanogui::Window ( div, "" );
			console_window->setLayout ( new nanogui::GroupLayout() );
			console = new nanogui::Console ( console_window );
			console->setSize ( {500, 50} );
			console->setFontSize ( 16 );
			
			//legend
			nanogui::Graph *graph = new nanogui::Graph ( m_window, "", nanogui::GraphType::GRAPH_LEGEND );
			graph->values().resize ( 10 );
			graph->values() << 1, 1, 1, 1, 1, 1, 1, 1, 1, 1;
			
			magbox_radius = Eigen::Vector3f ( 0.01f, 0.01f, 0.05f );
			cursor = Eigen::Vector3f ( 0.000f, 0.000f, 0.00f );
			cursor_radius = Eigen::Vector3f ( 0.0005f, 0.0005f, 0.01f );
			
		}
		
		nanogui::Window *m_window;
		nanogui::CheckBox *m_grid, *m_polar, *m_magbox, *show_inputs;
		nanogui::Label *magboxstate;
		nanogui::Console *console;
		std::string console_text = "";
		
		std::vector<int> selected_points;

		bool magmove;
		
		Eigen::Vector3f magbox;
		Eigen::Vector3f cursor;
		Eigen::Vector3f cursor_radius;
		Eigen::Vector3f magbox_radius;
		
		
};

float pt_scale = 30.0f;

class MagPlot : public nanogui::GLCanvas {
	public:
	
		MagPlot ( Widget *parent, const Eigen::Vector2i &w_size, SurfWindow &helper_window,
				  std::vector<std::pair<int, std::string>> *plotdata ) : textures ( plotdata ), widgets ( helper_window ) ,
			nanogui::GLCanvas ( parent, true ) {
			
			setSize ( w_size );
			setBackgroundColor ( nanogui::Color ( 0, 0, 0, 64 ) );
			setDrawBorder ( true );
			setVisible ( true );
			
			// init shaders
			m_pointShader = new nanogui::GLShader();
			m_pointShader->initFromFiles ( MAGBOX_SHADER_NAME, MAGBOX_VERT_FILE, MAGBOX_FRAG_FILE );
			// init shaders

			m_cursorShader = new nanogui::GLShader();
			m_cursorShader->initFromFiles ( BOX_SHADER_NAME, BOX_VERT_FILE, BOX_FRAG_FILE );
			
			reset_view ();
			refresh();
			
		}
		
		void refresh() {
		
			bounds_min = widgets.magbox - widgets.magbox_radius;
			bounds_max = widgets.magbox + widgets.magbox_radius;

			cursor_min = widgets.cursor - widgets.cursor_radius;
			cursor_max = widgets.cursor + widgets.cursor_radius;

			generate_points();
			
			/* Upload points to GPU */
			m_pointShader->bind();
			// std::cout << (std::vector<std::pair<int, std::string>> (*textures)[0]).first << std::endl;
			m_pointShader->uploadAttrib ( "texcoords", texcoords );
			m_pointShader->uploadAttrib ( "position", positions );
			m_pointShader->uploadAttrib ( "color", colors );
			
		}
		
		void generate_points() {
		
			size_t total_points = nn->codes.cols();
			
			m_pointCount = 0;
			
			positions.resize ( 3, total_points * 6 );
			colors.resize ( 3, total_points * 6 );
			texcoords.resize ( 3, total_points * 6 );
			
			widgets.selected_points.clear();

			for ( size_t i = 0; i < total_points; i++ ) {
			
				Eigen::VectorXf coords;
				int label;
				
				coords = nn->codes.col ( i );
				label = nn->codes_colors ( i );
				
				float x, y, z;
				
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
				
				Eigen::Vector3f pt = Eigen::Vector3f ( x / pt_scale, y / pt_scale, z / pt_scale );
				
				float textures_per_dim = ceil ( sqrtf ( nn->test_data.size() ) );
				
				float quad_size = 0.000001f * ( bounds_max[1] - bounds_min[1] );
				float radius = sqrtf ( 2 * quad_size );
				
				//linear index to texture coords:
				Eigen::Vector3f tex_pos = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
										  ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );
										  
				// check if in box
				if ( ( pt.array() >= bounds_min.array() ).all() && ( pt.array() <= bounds_max.array() ).all() ) {
				
					nanogui::Color c = nanogui::parula_lut[label];

					if (( pt.array() >= cursor_min.array() ).all() && ( pt.array() <= cursor_max.array() ).all()) {

						radius = 2 * sqrtf ( 2 * quad_size );
						widgets.selected_points.push_back(i);

					} else radius = sqrtf ( 2 * quad_size );
					
					// upper left corner
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 0, 1.0f / textures_per_dim, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( -radius, -radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 0, 0, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( -radius, radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 1 / textures_per_dim, 1 / textures_per_dim, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( radius, -radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 1 / textures_per_dim, 0, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( radius, radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 0, 0, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( -radius, radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
					texcoords.col ( m_pointCount ) = tex_pos + Eigen::Vector3f ( 1 / textures_per_dim, 1 / textures_per_dim, 0 );
					positions.col ( m_pointCount ) = pt + Eigen::Vector3f ( radius, -radius, 0 );
					colors.col ( m_pointCount ) = Eigen::Vector3f ( c.r(), c.g(), c.b() );
					m_pointCount++;
					
				}
				
			}
	
			//std::cout << "Mag: " << m_pointCount / 6 << " points" << std::endl;
			
		}
		
		void drawGL() override {
		
			eye = Eigen::Vector3f ( widgets.magbox[0], widgets.magbox[1], 1 );
			view = nanogui::lookAt ( Eigen::Vector3f ( eye ), Eigen::Vector3f ( widgets.magbox[0], widgets.magbox[1], 0 ),
									 Eigen::Vector3f ( 0, 1,
											 0 ) );
											 
			near = fmax(0.001f, eye[2] - bounds_max[2]);
			far = eye[2] - bounds_min[2];
			
			// std::cout << "near " << near << ", far: " << far << std::endl;
			
			// std::cout << "min" << std::endl;
			// std::cout << bounds_min << std::endl << std::endl;
			// std::cout << "max" << std::endl;
			// std::cout << bounds_max << std::endl << std::endl;
			
			float fH = fabs ( ( bounds_max[1] - bounds_min[1] ) / 2.0 * near );
			float fW = fabs ( ( bounds_max[0] - bounds_min[0] ) / 2.0 * near );
			
			proj = nanogui::frustum ( -fW, fW, -fH, fH, near, far );
			
			model.setIdentity();
			
			// Eigen::Quaternionf q = Eigen::AngleAxisf ( model_angle[0] * M_PI, Eigen::Vector3f::UnitX() )
			// 					   * Eigen::AngleAxisf ( model_angle[1] * M_PI,  Eigen::Vector3f::UnitY() )
			// 					   * Eigen::AngleAxisf ( model_angle[2] * M_PI, Eigen::Vector3f::UnitZ() );
			
			
			// model.block ( 0, 0, 3, 3 ) = q.matrix() * model.block ( 0, 0, 3, 3 );
			
			// // model = m_arcball.matrix() * model;
			model = nanogui::translate ( Eigen::Vector3f ( translation ) ) * model;
			
			// /* Render the point set */
			mvp = proj * view * model;
			
			glEnable( GL_POLYGON_SMOOTH );
			glEnable ( GL_LINE_SMOOTH );
			glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );
			glEnable(GL_MULTISAMPLE);
			glfwWindowHint(GLFW_SAMPLES, 4);

			if ( m_pointCount > 0 ) {
			
				m_pointShader->bind();
				
				glActiveTexture ( GL_TEXTURE0 );
				glBindTexture ( GL_TEXTURE_2D, ( std::vector<std::pair<int, std::string>> ( *textures ) [0] ).first );
				m_pointShader->setUniform ( "image", 0 );
				m_pointShader->setUniform ( "mvp", mvp );
				
				glPointSize ( 1 );
				glEnable ( GL_DEPTH_TEST );
				
				glEnable ( GL_BLEND );
				// glBlendFunc ( GL_SRC_ALPHA, GL_DST_ALPHA );
				glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				
				m_pointShader->drawArray ( GL_TRIANGLES, 0, m_pointCount );
				
				glDisable ( GL_BLEND );
				glDisable ( GL_DEPTH_TEST );
			}
			
			m_cursorShader->bind();
			m_cursorShader->setUniform ( "mvp", mvp );
			m_cursorShader->uploadAttrib ( "position", cursorline_positions );
			glEnable ( GL_BLEND );
			glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			m_cursorShader->drawArray ( GL_LINES, 0, m_cursorlineCount );
			glDisable ( GL_BLEND );
		}
		
		bool mouseMotionEvent ( const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers ) override {
		
			/* takes mouse position on screen and return ray in world coords */
			Eigen::Vector3f relative_pos = Eigen::Vector3f ( 2.0f * ( float ) ( p[0] - mPos[0] ) / ( float ) mSize[0] - 1.0f,
										   ( float ) ( -2.0f * ( p[1] - mPos[1] ) ) / ( float ) mSize[0] + 1.0f, 1.0f );
										   
			mouse_last_x = relative_pos[0];
			mouse_last_y = relative_pos[1];
			
			update_mouse_overlay();
			
			return true;
		}
		
		void update_mouse_overlay() {
		
			// ray casting
			Eigen::Vector3f ray_nds ( mouse_last_x, mouse_last_y, -1.0f );
			Eigen::Vector4f ray_clip ( mouse_last_x, mouse_last_y, -1.0f, -1.0f );
			Eigen::Vector4f ray_eye = proj.inverse() * ray_clip; ray_eye[2] = -1.0f; ray_eye[3] = 1.0f;
			Eigen::Vector4f cursorld = ( view.inverse() * ray_eye );
			
			widgets.cursor[0] = cursorld[0]; widgets.cursor[1] = cursorld[1]; widgets.cursor[2] = cursorld[2];
			drawbox ( cursorline_positions, m_cursorlineCount, widgets.cursor, widgets.cursor_radius );	
		}
		
		void reset_view () {
		
			translation = Eigen::Vector3f::Zero();
			model_angle = Eigen::Vector3f::Zero();
			
		}
		
		~MagPlot() {
		
			delete m_pointShader;
			delete m_cursorShader;
		}
		
		SurfWindow &widgets;
		
		// coords of points
		Eigen::MatrixXf positions;
		Eigen::MatrixXf texcoords;
		Eigen::MatrixXf colors;
		size_t m_cursorlineCount = 12 * 2;
		Eigen::MatrixXf cursorline_positions;		
		nanogui::GLShader *m_cursorShader = nullptr;
		nanogui::GLShader *m_pointShader = nullptr;

		std::vector<std::pair<int, std::string>> *textures;
		
		Eigen::Vector3f eye;
		float near, far;
		Eigen::Matrix4f view, proj, model, mvp;
		Eigen::Vector3f translation;
		Eigen::Vector3f model_angle;
		Eigen::Vector3f bounds_min, cursor_min;
		Eigen::Vector3f bounds_max, cursor_max;
		
		float mouse_last_x = -1, mouse_last_y = - 1;
		
		size_t m_pointCount = 1;
		
};

class SurfPlot : public nanogui::GLCanvas {

	public:
		SurfPlot ( Widget *parent, const Eigen::Vector2i &w_size, SurfWindow &helper_window ) : widgets ( helper_window ) ,
			nanogui::GLCanvas ( parent ) {
			using namespace nanogui;
			
			
			setSize ( w_size );
			
			setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			setDrawBorder ( true );
			
			// init shaders
			m_pointShader = new nanogui::GLShader();
			m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );
			m_gridShader = new nanogui::GLShader();
			m_gridShader->initFromFiles ( GRID_SHADER_NAME, GRID_VERT_FILE, GRID_FRAG_FILE );
			m_boxShader = new nanogui::GLShader();
			m_boxShader->initFromFiles ( BOX_SHADER_NAME, BOX_VERT_FILE, BOX_FRAG_FILE );
			
			refresh();
			setVisible ( true );
			
			// set default view
			reset_view();
			m_arcball.setSize ( w_size );
			
			drag_sensitivity = 5.0f;
			scroll_sensitivity = 10.0f;
			keyboard_sensitivity = 0.1f;
			
		}
		
		void reset_view ( int i = 0 ) {
		
			Eigen::Matrix3f mat;
			
			eye = Eigen::Vector3f ( 0, 0, 1 );
			
			if ( i == 0 ) {
			
				mat = Eigen::Matrix3f::Identity();
				translation = Eigen::Vector3f::Zero();
				model_angle = Eigen::Vector3f::Zero();
				
			}
			
			if ( i == 1 ) {
			
				mat = Eigen::Matrix3f::Identity();
				translation = Eigen::Vector3f ( 0, -0.97, -2.2 );
				model_angle = Eigen::Vector3f ( -0.46, 0, 2 );
				
			}
			
			if ( i == 2 ) {
			
				mat = Eigen::Matrix3f::Identity();
				translation = Eigen::Vector3f ( -0.575, -0.575, 0 );
				model_angle = Eigen::Vector3f ( 0, 0, 0 );
				
			}
			
			Eigen::Quaternionf q ( mat );
			m_arcball.setState ( q );
			
			widgets.magbox.setOnes();
			
		}
		
		void generate_points() {
		
			m_pointCount = nn->codes.cols();
			
			positions.resize ( 3, m_pointCount );
			colors.resize ( 3, m_pointCount );
			
			for ( size_t i = 0; i < m_pointCount; i++ ) {
			
				Eigen::VectorXf coords;
				int label;
				
				coords = nn->codes.col ( i );
				label = nn->codes_colors ( i );
				
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
				
				positions.col ( i ) << x / pt_scale, y / pt_scale, z / pt_scale;
				// std::cout << positions.col ( i ) << std::endl;
				nanogui::Color c = nanogui::parula_lut[label];
				
				if ( widgets.m_magbox->checked() ) { // mag box mode
				
					// check if in box
					if ( ( positions.col ( i ) [0] >= widgets.magbox[0] - widgets.magbox_radius[0] ) &&
							( positions.col ( i ) [0] <= widgets.magbox[0] + widgets.magbox_radius[0] ) &&
							( positions.col ( i ) [1] >= widgets.magbox[1] - widgets.magbox_radius[1] ) &&
							( positions.col ( i ) [1] <= widgets.magbox[1] + widgets.magbox_radius[1] ) &&
							( positions.col ( i ) [2] >= widgets.magbox[2] - widgets.magbox_radius[2] ) &&
							( positions.col ( i ) [2] <= widgets.magbox[2] + widgets.magbox_radius[2] ) )
							
						colors.col ( i ) << 2 * c.r(), 2 * c.g(), 2 * c.b();
					else
						colors.col ( i ) << 0.7 * c.r(), 0.7 * c.g(), 0.7 * c.b();
						
				}
				else
					colors.col ( i ) << c.r(), c.g(), c.b();
					
			}
			
		}
		
		bool mouseMotionEvent ( const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers ) override {
			// if ( !SurfPlot::mouseMotionEvent ( p, rel, button, modifiers ) )
			m_arcball.motion ( p );
			
			/* takes mouse position on screen and return ray in world coords */
			Eigen::Vector3f relative_pos = Eigen::Vector3f ( 2.0f * ( float ) ( p[0] - mPos[0] ) / ( float ) mSize[0] - 1.0f,
										   ( float ) ( -2.0f * ( p[1] - mPos[1] ) ) / ( float ) mSize[0] + 1.0f, 1.0f );
										   
			mouse_last_x = relative_pos[0];
			mouse_last_y = relative_pos[1];
			
			update_mouse_overlay();
			
			return true;
		}
		
		void update_mouse_overlay() {
		
			// ray casting
			Eigen::Vector3f ray_nds ( mouse_last_x, mouse_last_y, -1.0f );
			Eigen::Vector4f ray_clip ( mouse_last_x, mouse_last_y, -1.0f, -1.0f );
			Eigen::Vector4f ray_eye = proj.inverse() * ray_clip; ray_eye[2] = -1.0f; ray_eye[3] = 1.0f;
			Eigen::Vector4f magboxld = ( view.inverse() * ray_eye );
			
			if ( widgets.m_magbox->checked() && widgets.magmove ) {
			
				widgets.magbox[0] = magboxld[0]; widgets.magbox[1] = magboxld[1];
				widgets.magbox[2] = magboxld[2];
				//std::cout << "box: " << std::endl << widgets.magbox << std::endl;
				
			}
			
		}
		
		bool mouseButtonEvent ( const Eigen::Vector2i &p, int button, bool down, int modifiers ) override {
		
			// if ( !SurfPlot::mouseButtonEvent ( p, button, down, modifiers ) ) {
			if ( button == GLFW_MOUSE_BUTTON_1 )
				m_arcball.button ( p, down );
			// }
			
			return true;
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
			
				m_gridlineCount = 17 * 4 * 2;
				gridline_positions.resize ( 3, m_gridlineCount );
				
				size_t ii = 0;
				for ( float k = -2.0f; k <= 2.0f; k += 0.25f ) {
				
					gridline_positions.col ( ii + 0 ) << -k, -k, 0; gridline_positions.col ( ii + 1 ) << -k , k, 0; //L
					gridline_positions.col ( ii + 2 ) << k, -k , 0; gridline_positions.col ( ii + 3 ) << k, k, 0; //R
					gridline_positions.col ( ii + 4 ) << -k , -k , 0; gridline_positions.col ( ii + 5 ) << k, -k, 0; //B
					gridline_positions.col ( ii + 6 ) << -k, k, 0; gridline_positions.col ( ii + 7 ) << k, k, 0; //T
					
					// gridline_positions.col ( ii + 8 ) << 0, 0, 0; gridline_positions.col ( ii + 9 ) << 0, 0, k;
					// gridline_positions.col ( ii + 10 ) << k, 0, 0; gridline_positions.col ( ii + 11 ) << k, 0, k;
					// gridline_positions.col ( ii + 12 ) << 0, k, 0; gridline_positions.col ( ii + 13 ) << 0, k, k;
					// gridline_positions.col ( ii + 14 ) << k, k, 0; gridline_positions.col ( ii + 15 ) << k, k, k;
					
					// gridline_positions.col ( ii + 16 ) << 0, 0, k; gridline_positions.col ( ii + 17 ) << 0, k, k; //UL
					// gridline_positions.col ( ii + 18 ) << k, 0, k; gridline_positions.col ( ii + 19 ) << k, k, k; //UR
					// gridline_positions.col ( ii + 20 ) << 0, 0, k; gridline_positions.col ( ii + 21 ) << k, 0, k; //UB
					// gridline_positions.col ( ii + 22 ) << 0, k, k; gridline_positions.col ( ii + 23 ) << k, k, k; //UT
					
					ii += 4 * 2;
				}
				
				m_gridShader->bind();
				m_gridShader->uploadAttrib ( "position", gridline_positions );
			}
			
			if ( widgets.m_magbox->checked() ) {
			
				drawbox ( boxline_positions, m_boxlineCount, widgets.magbox, widgets.magbox_radius );
				m_boxShader->bind();
				m_boxShader->uploadAttrib ( "position", boxline_positions );
				
			}
		}
		
		// smooth keys - TODO: move to nanogui
		void process_keyboard() {
		
			// keyboard management
			/*	TODO: move to nanogui - need to modify keyboardEvent to allow smooth opeartion */
			// translation
			if ( glfwGetKey ( screen->glfwWindow(), 'A' ) == GLFW_PRESS ) translation[0] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'D' ) == GLFW_PRESS ) translation[0] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'S' ) == GLFW_PRESS ) translation[1] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'W' ) == GLFW_PRESS ) translation[1] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'E' ) == GLFW_PRESS ) translation[2] += 0.5 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'Q' ) == GLFW_PRESS ) translation[2] -= 0.5 * keyboard_sensitivity;
			
			if ( glfwGetKey ( screen->glfwWindow(), '9' ) == GLFW_PRESS ) { fov += 0.1f; fov = fmaxf ( fov, 0.1f ); };
			if ( glfwGetKey ( screen->glfwWindow(), '0' ) == GLFW_PRESS ) { fov -= 0.1f; fov = fminf ( fov, 180.0f ); }
			
			// rotation around x, y, z axes
			if ( glfwGetKey ( screen->glfwWindow(), 'Z' ) == GLFW_PRESS ) model_angle[1] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), 'C' ) == GLFW_PRESS ) model_angle[1] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_UP ) == GLFW_PRESS ) model_angle[0] -= 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_DOWN ) == GLFW_PRESS ) model_angle[0] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_LEFT ) == GLFW_PRESS ) model_angle[2] += 0.05 * keyboard_sensitivity;
			if ( glfwGetKey ( screen->glfwWindow(), GLFW_KEY_RIGHT ) == GLFW_PRESS ) model_angle[2] -= 0.05 * keyboard_sensitivity;
			
			if ( glfwGetKey ( screen->glfwWindow(), '1' ) == GLFW_PRESS )  reset_view ( 0 );
			if ( glfwGetKey ( screen->glfwWindow(), '2' ) == GLFW_PRESS )  reset_view ( 1 );
			if ( glfwGetKey ( screen->glfwWindow(), '3' ) == GLFW_PRESS )  reset_view ( 2 );
			
			if ( glfwGetKey ( screen->glfwWindow(), '[' ) == GLFW_PRESS )  {
			
				widgets.magbox_radius[2] -= 0.001f; widgets.magbox_radius[2] = fmaxf ( widgets.magbox_radius[2], 0.001f );
				
			}
			
			if ( glfwGetKey ( screen->glfwWindow(), ']' ) == GLFW_PRESS )  widgets.magbox_radius[2] += 0.005f;
			
			if ( glfwGetKey ( screen->glfwWindow(), 'O' ) == GLFW_PRESS ) {
			
				widgets.magbox_radius[0] -= 0.001f; widgets.magbox_radius[0] = fmaxf ( widgets.magbox_radius[0], 0.001f );
				widgets.magbox_radius[1] -= 0.001f; widgets.magbox_radius[1] = fmaxf ( widgets.magbox_radius[1], 0.001f );
				
			}
			
			if ( glfwGetKey ( screen->glfwWindow(), 'P' ) == GLFW_PRESS ) { widgets.magbox_radius[0] += 0.005f; widgets.magbox_radius[1] += 0.005f; }
			
			// std::cout << "transl " << translation << std::endl;
			// std::cout << "model " << model_angle << std::endl;
			// std::cout << "q " << m_arcball.matrix() << std::endl;
			
			update_mouse_overlay();
		}
		
		void drawGL() override {
		
			process_keyboard();
			
			/* Set up a perspective camera matrix */
			
			view = nanogui::lookAt ( eye, Eigen::Vector3f ( 0, 0, 0 ), Eigen::Vector3f ( 0, 1, 0 ) );
			near = 0.01;
			far = 100;
			float fH = std::tan ( fov / 360.0f * M_PI ) * near;
			float fW = fH * ( float ) mSize.x() / ( float ) mSize.y();
			proj = nanogui::frustum ( -fW, fW, -fH, fH, near, far );
			
			model.setIdentity();
			
			Eigen::Quaternionf q = Eigen::AngleAxisf ( model_angle[0] * M_PI, Eigen::Vector3f::UnitX() )
								   * Eigen::AngleAxisf ( model_angle[1] * M_PI,  Eigen::Vector3f::UnitY() )
								   * Eigen::AngleAxisf ( model_angle[2] * M_PI, Eigen::Vector3f::UnitZ() );
								   
								   
			model.block ( 0, 0, 3, 3 ) = q.matrix() * model.block ( 0, 0, 3, 3 );
			
			model = m_arcball.matrix() * model;
			model = nanogui::translate ( Eigen::Vector3f ( translation ) ) * model;
			
			/* Render the point set */
			mvp = proj * view * model;
			
			update_mouse_overlay();
			
			m_pointShader->bind();
			m_pointShader->setUniform ( "mvp", mvp );
			
			glEnable ( GL_PROGRAM_POINT_SIZE );
			//glEnable ( GL_DEPTH_TEST );

			// antialiasing
			glEnable( GL_POLYGON_SMOOTH );
			glEnable ( GL_LINE_SMOOTH );
			glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );
			glEnable(GL_MULTISAMPLE);
			glfwWindowHint(GLFW_SAMPLES, 4);

			m_pointShader->drawArray ( GL_POINTS, 0, m_pointCount );
			
			if ( widgets.m_grid->checked() ) {
				m_gridShader->bind();
				m_gridShader->setUniform ( "mvp", mvp );
				glEnable ( GL_BLEND );
				glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				m_gridShader->drawArray ( GL_LINES, 0, m_gridlineCount );
				glDisable ( GL_BLEND );
			}
			
			if ( widgets.m_magbox->checked() ) {
			
				m_boxShader->bind();
				m_boxShader->setUniform ( "mvp", mvp );
				glEnable ( GL_BLEND );
				glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				m_boxShader->drawArray ( GL_LINES, 0, m_boxlineCount );
				glDisable ( GL_BLEND );
				
			}
			
			//glDisable ( GL_DEPTH_TEST );
			glDisable ( GL_PROGRAM_POINT_SIZE );
			
			// perf stats
			update_FPS ( *graph_data );
			
		}
		
		~SurfPlot() {
		
			delete m_pointShader;
			delete m_gridShader;
			delete m_boxShader;
			
		}
		
		size_t m_pointCount = 1;
		size_t m_gridlineCount = 12 * 2;
		size_t m_boxlineCount = 12 * 2;
		
		// shaders
		nanogui::GLShader *m_pointShader = nullptr;
		nanogui::GLShader *m_gridShader = nullptr;
		nanogui::GLShader *m_boxShader = nullptr;
		
		nanogui::Arcball m_arcball;
		
		Eigen::VectorXf *graph_data;
		
		Eigen::Vector3f translation;
		Eigen::Vector3f model_angle;
		
		// coords of points
		Eigen::MatrixXf positions;
		Eigen::MatrixXf gridline_positions;
		Eigen::MatrixXf boxline_positions;
		Eigen::MatrixXf colors;
		
		Eigen::Vector3f eye;
		Eigen::Matrix4f view, proj, model, mvp;
		
		SurfWindow &widgets;
		
		float mouse_last_x = -1, mouse_last_y = -1;
		
		float fov = 60;
		float near, far;
		float drag_sensitivity, scroll_sensitivity, keyboard_sensitivity;
		
		
};

#endif
