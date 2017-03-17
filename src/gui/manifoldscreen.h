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

#define DEF_WIDTH 3560
#define DEF_HEIGHT 2076
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
			
			images = new nanogui::Window ( this, "images" );
			images->setLayout ( new nanogui::VGroupLayout () );
			images->setPosition ( { 165 / screen_scale, 5 / screen_scale } );
			
			nanogui::ImagePanel *inp = new nanogui::ImagePanel ( images, 50 / screen_scale, 2 / screen_scale, 2 / screen_scale, {10, batch_size / 10} );
			inp->setImages ( xs );
			nanogui::ImagePanel *out = new nanogui::ImagePanel ( images, 50 / screen_scale, 2 / screen_scale, 2 / screen_scale, {10, batch_size / 10} );
			out->setImages ( ys );
			
			nanogui::Window *plot = new nanogui::Window ( this, "plot" );
			plot->setPosition ( { 15 / screen_scale, 275 / screen_scale } );
			plot->setLayout ( new nanogui::GroupLayout() );
			
			nanogui::Window *plotmag = new nanogui::Window ( this, "plot mag" );
			plotmag->setPosition ( { 1470 / screen_scale, 15 / screen_scale } );
			plotmag->setLayout ( new nanogui::GroupLayout() );
			
			/* * * * * * * * * * * */
			
			
			// wait until nn is ready
			while ( ! ( nn->isready() ) )
				usleep ( 10000 );
				
				
			mCanvas_helper = new SurfWindow ( plot, "" );
			mCanvas_helper->setSize ( {1400, 50} );
			mCanvas_helper->setLayout ( new nanogui::GroupLayout ( 15, 0, 0, 0 ) );
			
			/* FPS GRAPH */
			graph_fps = new nanogui::Graph ( mCanvas_helper->m_window, "" );
			graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
			graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			
			graph_loss = new nanogui::Graph ( mCanvas_helper->m_window );
			graph_loss->setFooter ( "loss" );
			graph_loss->setGraphColor ( nanogui::Color ( 192, 160, 0, 255 ) );
			graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			
			//CPU graph
			graph_cpu = new nanogui::Graph ( mCanvas_helper->m_window, "" );
			graph_cpu->values().resize ( cpu_util.size() );
			graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
			graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			
			// FlOP/s
			graph_flops = new nanogui::Graph ( mCanvas_helper->m_window, "" );
			graph_flops->values().resize ( cpu_flops.size() );
			graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
			graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			
			// B/s
			graph_bytes = new nanogui::Graph ( mCanvas_helper->m_window, "" );
			graph_bytes->values().resize ( cpu_reads.size() );
			graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
			graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			
			// nanogui::Window *graphs = new nanogui::Window ( mCanvas_helper, "" );
			// graphs->setLayout ( new nanogui::VGroupLayout() );
			
			
			
			mCanvas = new SurfPlot ( plot, {1400, 1400}, *mCanvas_helper );
			mCanvas->graph_data = graph_fps->values_ptr();
			mCanvas->setBackgroundColor ( {100, 100, 100, 64} );
			
			std::cout << nn->train_data.size() << std::endl;
			std::cout << nn->train_data[0].x.size() << std::endl;
			
			plotdata.resize ( 1 );
			
			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;
			
			size_t sqr_dim = ceil ( sqrtf ( nn->test_data.size() ) );
			std::cout << sqr_dim *sqr_dim << " " << ceil ( sqr_dim ) << std::endl;
			
			rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size );
			Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );
			
			for ( size_t i = 0; i < nn->test_data.size(); i++ ) {
			
				// std::cout << i << std::endl;
				float_image =  nn->test_data[i].x;
				float_image.resize ( image_size, image_size );
				float_image *= 255.0f;
				
				rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size,
								   image_size ) = float_image.cast<unsigned char>();
								   
			}
			
			plotdata[0] = ( std::pair<int, std::string> ( nvgCreateImageA ( nvgContext(),
							sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST, ( unsigned char * ) rgba_image.data() ), "" ) );
							
			mCanvasMag = new MagPlot ( plotmag, {1960, 1960}, *mCanvas_helper, &plotdata );
			mCanvasMag->setBackgroundColor ( {100, 100, 100, 64} );
			
			drawAll();
			setVisible ( true );
			
			glfwSwapInterval ( vsync );
			
			performLayout();
			resizeEvent ( { glfw_window_width, glfw_window_height } );
			
			
		}
		
		
		virtual void drawContents() {
		
			refresh();
			images->setVisible ( mCanvas_helper->show_inputs->checked() );
			
		}
		
		void refresh() {
		
			if ( mCanvas_helper->show_inputs->checked() ) {
			
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
			}
			
			if ( mCanvas ) {
				mCanvas->refresh();
				if ( mCanvas->graph_data->size() > 0 )
					graph_fps->setHeader ( string_format ( "%.1f FPS", mCanvas->graph_data->mean() ) );
			}
			
			if ( mCanvas_helper ) {
			
				mCanvas_helper->console_text = string_format (
												   "Points 0 : %zu, Points 1: %zu, Selected: %zu\n"
												   "Box = (%.4f, %.4f, %.4f) +/- (%.4f, %.4f, %.4f)\n"
												   "Raycast 0: m (%.4f, %.4f), near: %.3f, far: %.3f, cursor 0: (%.2f, %.2f, %.2f)\n"
												   "Raycast 1: m (%.4f, %.4f), near: %.3f, far: %.3f, cursor 1: (%.2f, %.2f, %.2f)\n"
												   "FOV 0: %f, Cam 0 = (%.2f, %.2f, %.2f), Cam 1 = (%.2f, %.2f, %.2f)\n"
												   "Translation = (%.2f, %.2f, %.2f), Angle = (%.2f, %.2f, %.2f)\n",
												   
												   mCanvas->m_pointCount, mCanvasMag->m_pointCount/6, mCanvas_helper->selected_points.size(),
												   mCanvas_helper->magbox[0], mCanvas_helper->magbox[1], mCanvas_helper->magbox[2],
												   mCanvas_helper->magbox_radius[0], mCanvas_helper->magbox_radius[1], mCanvas_helper->magbox_radius[2],
												   mCanvas->mouse_last_x, mCanvas->mouse_last_y, mCanvas->near, mCanvas->far,
												   mCanvas_helper->magbox[0], mCanvas_helper->magbox[0], mCanvas_helper->magbox[0],
												   mCanvasMag->mouse_last_x, mCanvasMag->mouse_last_y, mCanvasMag->near, mCanvasMag->far,
												   mCanvas_helper->cursor[0], mCanvas_helper->cursor[1], mCanvas_helper->cursor[2],
												   mCanvas->fov, mCanvas->eye[0], mCanvas->eye[1], mCanvas->eye[2],
												   mCanvasMag->eye[0], mCanvasMag->eye[1], mCanvasMag->eye[2],
												   mCanvas->translation[0], mCanvas->translation[1], mCanvas->translation[2],
												   mCanvas->model_angle[0], mCanvas->model_angle[1], mCanvas->model_angle[2] );
				
				std::string selected_contents = "";

				for (size_t i = 0; i < mCanvas_helper->selected_points.size(); i++) {

					selected_contents += std::to_string(mCanvas_helper->selected_points[i]);
					if (i != mCanvas_helper->selected_points.size() - 1) selected_contents += ", ";
				}

				mCanvas_helper->console->setValue ( mCanvas_helper->console_text + selected_contents );
				
				if ( nn->clock ) {
					nn->clock = false;
					update_graph ( graph_loss, nn->current_loss );
					update_graph ( graph_cpu, cpu_util, 1000.0f, "ms" );
					update_graph ( graph_flops, cpu_flops, 1.0f, "GF/s" );
					update_graph ( graph_bytes, cpu_reads, 1.0f, "MB/s" );
				}
			}
			
			if ( mCanvasMag )
				mCanvasMag->refresh();
				
		}
		
		/* event handlers */
		
		virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {
		
			if ( action ) {
				switch ( key ) {
				
					case GLFW_KEY_SPACE:
						nn->pause = !nn->pause;
						return true;
						
					case GLFW_KEY_N:
						if ( nn->pause ) nn->step = true;
						return true;
						
					case GLFW_KEY_ESCAPE:
						nn->quit = true;
						setVisible ( false );
						return true;
						
					case GLFW_KEY_B:
						mCanvas_helper->m_magbox->setChecked ( !mCanvas_helper->m_magbox->checked() );
						return true;
						
					case GLFW_KEY_L:
						mCanvas_helper->m_polar->setChecked ( !mCanvas_helper->m_polar->checked() );
						return true;
						
					case GLFW_KEY_TAB:
						mCanvas_helper->magmove = !mCanvas_helper->magmove;
						mCanvas_helper->magboxstate->setCaption ( string_format ( "Mag box locked: %d", ! ( mCanvas_helper->magmove ) ) );
						return true;
						
						
				}
			}
			
			return Screen::keyboardEvent ( key, scancode, action, modifiers );
		}
		
		virtual bool resizeEvent ( const Eigen::Vector2i &size ) {
		
			UNUSED ( size );
			performLayout();
			
			return true;
			
		}
		
		~GUI() { /* free resources */}
		
	protected:
	
		int glfw_window_width, glfw_window_height;
		bool vsync;
		
		SurfWindow *mCanvas_helper;
		SurfPlot *mCanvas;
		MagPlot *mCanvasMag;
		
		nanogui::Window *images;
		
		nanogui::Graph *graph_fps, *graph_loss, *graph_cpu, *graph_flops, *graph_bytes;
		
		using imagesDataType = std::vector<std::pair<int, std::string>>;
		imagesDataType xs, ys, plotdata;
		
};

#endif