#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <colors.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/layout.h>
#include <nanogui/imagepanel.h>

// nvgCreateImageA
#include <gl/tex.h>

#define DEF_WIDTH 1705
#define DEF_HEIGHT 1221
#define SCREEN_NAME "AE"

class GUI : public nanogui::Screen {

	public:
	
		GUI ( ) :
		
			nanogui::Screen ( Eigen::Vector2i ( DEF_WIDTH / screen_scale, DEF_HEIGHT / screen_scale ), SCREEN_NAME ),
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
			
			nanogui::ImagePanel *inp = new nanogui::ImagePanel ( images, 50 / screen_scale, 2 / screen_scale, 2 / screen_scale, {10, batch_size / 10} );
			inp->setImages ( xs );
			nanogui::ImagePanel *out = new nanogui::ImagePanel ( images, 50 / screen_scale, 2 / screen_scale, 2 / screen_scale, {10, batch_size / 10} );
			out->setImages ( ys );
			
			nanogui::Window *plot = new nanogui::Window ( this, "" );
			plot->setPosition ( { 585 / screen_scale, 15 / screen_scale } );
			plot->setLayout ( new nanogui::GroupLayout() );
			
			
			/* * * * * * * * * * * */
			
			// wait until nn is ready
			while ( ! ( nn->isready() ) )
				usleep ( 10000 );
				
			mCanvas = new SurfPlot ( plot, {1070 / screen_scale, 1070 / screen_scale} );
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

#endif