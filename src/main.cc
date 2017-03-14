/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-13 19:30:13
*/

#include <thread>
#include <unistd.h>

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/layout.h>
#include <nanogui/imagepanel.h>

//helpers
#include <utils.h>

//for NN
#include <io/import.h>
#include <nn/nn_utils.h>
#include <nn/layers.h>
#include <nn/nn.h>

// nvgCreateImageA
#include <gl/tex.h>

NN *nn;

#define DEF_WIDTH 1705
#define DEF_HEIGHT 1221
#define SCREEN_NAME "AE"

const size_t batch_size = 100;
const size_t image_size = 28;

// Surfplot
#include <nanogui/glcanvas.h>
#include <nanogui/checkbox.h>
#define POINT_SHADER_NAME "point_shader"
#define POINT_FRAG_FILE "./src/glsl/surf_point.f.glsl"
#define POINT_VERT_FILE "./src/glsl/surf_point.v.glsl"

#define GRID_SHADER_NAME "grid_shader"
#define GRID_FRAG_FILE "./src/glsl/surf_grid.f.glsl"
#define GRID_VERT_FILE "./src/glsl/surf_grid.v.glsl"

// FPS
#include "fps.h"
#include <rand/pcg32.h>

nanogui::Screen *screen;

class SurfPlot : public nanogui::GLCanvas {
	public:
		SurfPlot ( Widget *parent, const Eigen::Vector2i &w_size ) : nanogui::GLCanvas ( parent ) {
			using namespace nanogui;
			
			/* FPS GRAPH */
			m_window = new nanogui::Window ( this, "" );
			m_window->setPosition ( Eigen::Vector2i ( 5, 55 ) );
			m_window->setLayout ( new nanogui::GroupLayout() );
			
			m_gridCheckBox = new nanogui::CheckBox ( m_window, "Grid" );
			m_gridCheckBox->setCallback ( [&] ( bool ) { refresh(); } );
			
			m_gridCheckBox->setChecked ( true );
			
			/* FPS GRAPH */
			graph_fps = add<nanogui::Graph> ( "" );
			graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
			graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
			
			setBackgroundColor ( nanogui::Color ( 0, 0, 0, 8 ) );
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
				
				x = coords[0];
				y = coords[1];
				
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
			if ( m_gridCheckBox->checked() ) {
			
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
			
			if ( glfwGetKey ( screen->glfwWindow(), '1' ) == GLFW_PRESS ) viewAngle += 0.05;
			if ( glfwGetKey ( screen->glfwWindow(), '2' ) == GLFW_PRESS ) viewAngle -= 0.05;
			
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
			
			view = nanogui::lookAt ( Eigen::Vector3f ( 0, 0, 2 ), Eigen::Vector3f ( 0, 0, 0 ), Eigen::Vector3f ( 0, 1, 0 ) );
			const float near = 0.01, far = 100;
			float fH = std::tan ( viewAngle / 360.0f * M_PI ) * near;
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
			
			bool drawGrid = m_gridCheckBox->checked();
			
			if ( drawGrid ) {
				m_gridShader->bind();
				m_gridShader->setUniform ( "mvp", mvp );
				glEnable ( GL_BLEND );
				glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				m_gridShader->drawArray ( GL_LINES, 0, m_lineCount );
				glDisable ( GL_BLEND );
			}
			
			// perf stats
			update_FPS ( graph_fps );
			
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
		nanogui::Window *m_window;
		nanogui::CheckBox *m_gridCheckBox;
		nanogui::Graph *graph_fps;
		nanogui::Arcball m_arcball;
		
		Eigen::Vector3f translation;
		Eigen::Vector3f model_angle;
		
		// coords of points
		Eigen::MatrixXf positions;
		Eigen::MatrixXf line_positions;
		Eigen::MatrixXf colors;
		
		float viewAngle = 30;
		float drag_sensitivity, scroll_sensitivity, keyboard_sensitivity;
		
		
};

class GUI : public nanogui::Screen {

	public:
	
		GUI ( ) :
			nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH, DEF_HEIGHT ), SCREEN_NAME ),
			vsync ( true ) { init(); }
			
		void init() {
		
			/* get physical GLFW screen size */
			glfwGetWindowSize ( glfwWindow(), &glfw_window_width, &glfw_window_height );
			
			// get GL capabilities info
			printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
			printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
			printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );
			
			/* * create widgets  * */
			
			//make image placeholders
			for ( size_t i = 0; i < batch_size; i++ ) {
			
				xs.emplace_back ( std::pair<int, std::string> ( nvgCreateImageA ( nvgContext(),
								  image_size, image_size, NVG_IMAGE_NEAREST, ( unsigned char * ) nullptr ), "" ) );
								  
				ys.emplace_back ( std::pair<int, std::string> ( nvgCreateImageA ( nvgContext(),
								  image_size, image_size, NVG_IMAGE_NEAREST, ( unsigned char * ) nullptr ), "" ) );
								  
			}
			
			nanogui::Window *images = new nanogui::Window ( this, "images" );
			images->setLayout ( new nanogui::GroupLayout () );
			
			nanogui::ImagePanel *inp = new nanogui::ImagePanel ( images, 50, 2, 2, {10, batch_size / 10} );
			inp->setImages ( xs );
			nanogui::ImagePanel *out = new nanogui::ImagePanel ( images, 50, 2, 2, {10, batch_size / 10} );
			out->setImages ( ys );
			
			nanogui::Window *plot = new nanogui::Window ( this, "" );
			plot->setPosition ( { 585, 15 } );
			plot->setLayout ( new nanogui::GroupLayout() );
			
			
			/* * * * * * * * * * * */
			
			// wait until nn is ready
			while ( ! ( nn->isready() ) )
				usleep ( 10000 );
				
			mCanvas = new SurfPlot ( plot, {1070, 1070} );
			mCanvas->setBackgroundColor ( {100, 100, 100, 32} );
			
			nanogui::Graph *graph = new nanogui::Graph ( plot, "", nanogui::GraphType::GRAPH_LEGEND );
			graph->values().resize ( 10 );
			graph->values() << 1, 1, 1, 1, 1, 1, 1, 1, 1, 1;
			
			drawAll();
			setVisible ( true );
			
			glfwSwapInterval ( vsync );
			
			performLayout();
			resizeEvent ( { glfw_window_width, glfw_window_height } );
			
			
		}
		
		
		virtual void drawContents() {
		
			refresh();
			
		}
		
		void refresh() {
		
			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;
			rgba_image.resize ( image_size, image_size );
			
			// xs
			for ( size_t i = 0; i < nn->batch_size; i++ ) {
			
				Eigen::MatrixXf float_image = nn->layers[0]->x.col ( i );
				
				float_image.resize ( image_size, image_size );
				float_image *= 255.0f;
				
				rgba_image = float_image.cast<unsigned char>();
				nvgUpdateImage ( nvgContext(), xs[i].first, ( unsigned char * ) rgba_image.data() );
				
				
			}
			
			// ys
			for ( size_t i = 0; i < nn->batch_size; i++ ) {
			
				Eigen::MatrixXf float_image = nn->layers.back()->y.col ( i );
				
				float_image.resize ( image_size, image_size );
				float_image *= 255.0f;
				
				rgba_image = float_image.cast<unsigned char>();
				nvgUpdateImage ( nvgContext(), ys[i].first, ( unsigned char * ) rgba_image.data() );
				
			}
			
			mCanvas->refresh();
			
		}
		
		/* event handlers */
		
		virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {
		
			/* process subcomponents */
			if ( Screen::keyboardEvent ( key, scancode, action, modifiers ) )
				return true;
				
			if ( key == GLFW_KEY_SPACE && action == GLFW_PRESS )
				nn->pause = !nn->pause;
				
			if ( key == GLFW_KEY_N && action == GLFW_PRESS ) {
			
				if ( nn->pause )
					nn->step = true;
					
			}
			
			/* close */
			if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS ) {
			
				nn->quit = true;
				setVisible ( false );
				
				return true;
			}
			
			return false;
		}
		
		virtual bool resizeEvent ( const Eigen::Vector2i &size ) {
		
			UNUSED ( size );
			performLayout();
			
			printf ( "%d %d\n", size[0], size[1] );
			return true;
			
		}
		
		~GUI() { /* free resources */}
		
	protected:
	
		int glfw_window_width, glfw_window_height;
		bool vsync;
		
		std::vector<Eigen::VectorXf *> graph_data;
		SurfPlot *mCanvas;
		
		using imagesDataType = std::vector<std::pair<int, std::string>>;
		imagesDataType xs, ys;
		
};

int compute() {

	// TODO: be able to change batch size, learning rate and decay dynamically
	// serialization
	
	double learning_rate = 5 * 1e-4;
	float decay = 0.0f;
	
	std::vector<int> layer_sizes = {image_size * image_size, 100, 100, 100, 25, 3, 25, 100, 100, 100, image_size * image_size};
	
	nn = new NN ( batch_size, decay, AE );
	std::cout << "nn new" << std::endl;
	
	nn->layers.push_back ( new Linear ( layer_sizes[0], layer_sizes[1], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[1], layer_sizes[1], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[1], layer_sizes[2], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[2], layer_sizes[2], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[2], layer_sizes[3], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[3], layer_sizes[3], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[3], layer_sizes[4], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[4], layer_sizes[4], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[4], layer_sizes[5], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[5], layer_sizes[5], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[5], layer_sizes[6], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[6], layer_sizes[6], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[6], layer_sizes[7], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[7], layer_sizes[7], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[7], layer_sizes[8], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[8], layer_sizes[8], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[8], layer_sizes[9], batch_size ) );
	nn->layers.push_back ( new ReLU ( layer_sizes[9], layer_sizes[9], batch_size ) );
	
	nn->layers.push_back ( new Linear ( layer_sizes[9], layer_sizes[10], batch_size ) );
	nn->layers.push_back ( new Sigmoid ( layer_sizes[10], layer_sizes[10], batch_size ) );
	
	//[60000, 784]
	std::deque<datapoint> train_data =
		MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
		
	//[10000, 784]
	std::deque<datapoint> test_data =
		MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );
		
	nn->testcode ( train_data );
	
	std::cout << "nn ready" << std::endl;
	nn->setready();
	
	for ( size_t e = 0; true; e++ ) {
	
		nn->train ( train_data, learning_rate, train_data.size() / batch_size );
		nn->testcode ( train_data );
		
		if ( nn->quit ) break;
		
		printf ( "Epoch %3lu: Loss: %.2f\n", e + 1, ( float ) nn->test ( test_data ) );
		
	}
	
	nn->quit = true;
	
	//should wait for GL to finish first before deleting nn
	delete nn; return 0;
	
}

int main ( int /* argc */, char ** /* argv */ ) {

	try {
	
		/* init GUI */
		nanogui::init();
		
		// launch a compute thread
		std::thread compute_thread ( compute );
		
		// wait until nn is allocated
		while ( ! nn ) {
			std::cout << "main" << std::endl;
			usleep ( 1000 );
		}
		
		screen = new GUI();
		
		nanogui::mainloop ( 1 );
		
		delete screen;
		nanogui::shutdown();
		compute_thread.join();
		
	}
	catch ( const std::runtime_error &e ) {
	
		std::string error_msg = std::string ( "Caught a fatal error: " ) + std::string ( e.what() );
		return -1;
		
	}
	
	return 0;
	
}
