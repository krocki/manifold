#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/serializer/core.h>
#include <nanogui/serializer/opengl.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>

// helpers
#include <utils.h>
#include <colors.h>
#include <compute/functions.h>

#include "aux.h"
#include "perf.h"
#include "fps.h"

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glmanifold.h>
#include <gui/glplothelper.h>
#include <gui/nnview.h>

#include <gl/tex.h>

#define DEF_WIDTH 1300
#define DEF_HEIGHT 1120
#define SCREEN_NAME "GAN"
#define RESIZABLE true
#define FULLSCREEN false
#define COLOR_BITS 8
#define ALPHA_BITS 8
#define DEPTH_BITS 24
#define STENCIL_BITS 8
#define MSAA_SAMPLES 4
#define GL_MAJOR 3
#define GL_MINOR 3
#define VSYNC true
#define AUTOSIZE false
#define SIZE_RATIO 2.0f/3.0f

class GUI : public nanogui::Screen {

  public:

	GUI ( ) :

		nanogui::Screen ( { DEF_WIDTH , DEF_HEIGHT  }, SCREEN_NAME, RESIZABLE, FULLSCREEN, COLOR_BITS, ALPHA_BITS, DEPTH_BITS,
	STENCIL_BITS, MSAA_SAMPLES, GL_MAJOR, GL_MINOR, VSYNC, AUTOSIZE, SIZE_RATIO ) { init(); }

	void init() {

		// get GL capabilities info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		// init data
		plot_data = new PlotData();

		float box_size = 10.0f;

		generate_cube ( plot_data->c_indices, plot_data->c_vertices, plot_data->c_colors, {0, 0, 0}, box_size, {0.5f, 0.5f, 0.5f} );
		generate_mesh ( plot_data->m_indices, plot_data->m_vertices, plot_data->m_texcoords, plot_data->m_colors, {0, 0, -box_size},
		                box_size * 2 );

		plot_data->updated();

		// gui elements
		root = new nanogui::Window ( this, "" );
		root->setLayout ( new nanogui::VGroupLayout ( 5 ) );

		std::string t = return_current_time_and_date ( "%y%m%d_%H%M%S" );

		plots.push_back ( new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, true, true, true, false,
		                             mGLFWWindow, mNVGContext, 80.0f, { -15.2f, -15.8f, -15.0f}, { 0.63, 1.895f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2},
		                             false, 0, string_format ( "%s_plot0", t.c_str() ) ) );
		// plots.push_back(new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, true, true, true, false, mGLFWWindow, mNVGContext, 6.0f, { -5.0f, -8.0f, -1.0f}, { 0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot0", t.c_str())));

		plots.push_back ( new Plot ( root, "top frustum", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 1, plot_data, true, false, false,
		                             true, mGLFWWindow, mNVGContext, 66.0f, {0.0f, 35.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, {box_size * 2, box_size * 2, box_size * 2},
		                             false, 0, string_format ( "%s_plot_top", t.c_str() ) ) );

		plots.push_back ( new Plot ( root, "front ortho", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, true, false, false,
		                             false, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2},
		                             true, 0, string_format ( "%s_plot_ortho", t.c_str() ) ) );

		int number_of_cameras = plots.size();

		plot_data->e_vertices.resize ( 3, 2 * number_of_cameras );
		plot_data->r_vertices.resize ( 3, 2 * number_of_cameras );
		plot_data->e_colors.resize ( 3, 2 * number_of_cameras );

		plot_data->e_colors.col ( 0 ) << 1, 0, 0; plot_data->e_colors.col ( 1 ) << 1, 0, 0;
		plot_data->e_colors.col ( 2 ) << 0, 1, 0; plot_data->e_colors.col ( 3 ) << 0, 1, 0;
		plot_data->e_colors.col ( 4 ) << 0, 0, 1; plot_data->e_colors.col ( 5 ) << 0, 0, 1;

		// share shader data
		for ( int i = 0; i < number_of_cameras; i++ )
			plots[i]->master_pointShader = plots[0]->m_pointShader;

		// DATA
		// train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
		// test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

		//train_data = CIFARImporter::importFromFile("data/cifar-100-binary/train.bin", 2);

		// train_data = CIFARImporter::importFromFiles ( {
		// 	"data/cifar-10-batches-bin/data_batch_1.bin",
		// 	"data/cifar-10-batches-bin/data_batch_2.bin",
		// 	"data/cifar-10-batches-bin/data_batch_3.bin",
		// 	"data/cifar-10-batches-bin/data_batch_4.bin",
		// 	"data/cifar-10-batches-bin/data_batch_5.bin"
		// }, 1 );

		train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte",
		             "data/mnist/train-labels-idx1-ubyte" );

		reconstruction_data.resize ( train_data.size() );
		// TODO
		sample_reconstruction_data.resize ( 10000 );
		for ( int i = 0; i < sample_reconstruction_data.size();
		        i++ ) sample_reconstruction_data[i].x.resize ( train_data[0].x.size() );


		update_data_textures();

		// todo: set/save layout (including dynamically created widgets)
		// be able to load everything

		drawAll();
		setVisible ( true );

		/* widgets */
		makeWidgets();

		nnview = new NNView ( this, mNVGContext, plot_data, large_image_view, large_tex_view );
		nnview->setFixedSize ( {DEF_WIDTH, 760} );
		nnview->setPosition ( {3, 3} );

		performLayout();

		resizeEvent ( {DEF_WIDTH, DEF_HEIGHT} );

	}

	void save ( nanogui::Serializer &s ) const {

		s.set ( "position", mPos );
		s.set ( "size", mSize );
		s.set ( "fixedSize", mFixedSize );
		s.set ( "visible", mVisible );
		s.set ( "enabled", mEnabled );
		s.set ( "focused", mFocused );
		s.set ( "tooltip", mTooltip );
		s.set ( "fontSize", mFontSize );
		s.set ( "cursor", ( int ) mCursor );
	}

	void saveScreenShot ( int premult, const char *name ) {

		int w = mFBSize[0];
		int h = mFBSize[1];

		screenshot ( 0, 0, w, h, premult, name );

	}

	void saveScreenShotCropped ( int premult, const char *name ) {

		int w = mFBSize[0];
		int h = mFBSize[1];
		int x = 0;
		int y = h - w / 3;
		h = w / 3;
		w = w / 3;

		screenshot ( x, y, w, h, premult, name );

	}

	virtual void drawContents() {

		if ( nn && discriminator ) {
			if ( plot_data ) {

				if ( plot_data->checksum != local_data_checksum ) {

					if ( train_data[0].rgba ) {
						plot_data->update_nn_matrix_textures ( nn, mNVGContext, GL_RGBA );
						plot_data->update_disc_matrix_textures ( discriminator, mNVGContext, GL_RGBA );

					} else {
						plot_data->update_nn_matrix_textures ( nn, mNVGContext, GL_RED );
						plot_data->update_disc_matrix_textures ( discriminator, mNVGContext, GL_RED );
					}

					plot_data->update_gan_train_data ( gan_train_data, mNVGContext );
					plot_data->update_reconstructions ( reconstruction_data, sample_reconstruction_data, mNVGContext, train_data[0].image_w,
					                                    train_data[0].rgba );

					nnview->update_matrices();

					local_data_checksum = plot_data->checksum;

				}

			}

			mProgress->setValue ( nn->epoch_progress );
		}
	}

	/* event handlers */
	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		//todo: logf(type, "...", );

		printf ( "keyboardEvent: key %i scancode %i (%s), action %i mods %i\n", key, scancode, glfwGetKeyName ( key, scancode ),
		         action, modifiers );

		if ( action ) {

			switch ( key ) {

			case GLFW_KEY_ESCAPE:

				saveScreenShotCropped ( false, "dump.png" );
				setVisible ( false );
				nn->pause = false;
				nn->quit = true;
				return true;

			case GLFW_KEY_F12:

				std::string t = return_current_time_and_date ( "%y%m%d_%H%M%S" );
				saveScreenShot ( false, string_format ( "./snapshots/screens/screen_%s_%08d.png", t.c_str(),
				                                        completed_frames ).c_str() );
				return true;

			}

		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual void frame_completed_event() {

		int interval = 250;

		if ( completed_frames % interval == 0 ) {
			std::string t = return_current_time_and_date ( "%y%m%d_%H%M%S" );
			// saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
		}

		completed_frames++;

		// if ( nn ) {
		// 	if ( nn->clock ) {
		// 		nn->clock = false;
		// 		update_graph ( graph_loss, nn->current_loss );
		// 		update_graph ( graph_cpu, cpu_util, 1000.0f, "ms" );
		// 		update_graph ( graph_flops, cpu_flops, 1.0f, "GF/s" );
		// 		update_graph ( graph_bytes, cpu_reads, 1.0f, "MB/s" );
		// 	}
		// }

	};

	void update_data_textures() {

		//plot_data->load_data_textures(train_data, mNVGContext);
		plot_data->load_input_data_textures ( train_data, mNVGContext, train_data[0].image_w, train_data[0].rgba );

	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		// 8 because of VGroupLayout(5) spacing
		// for ( int i = 0; i < ( int ) plots.size(); i++ ) {
		// 	plots[i]->setSize ( {size[0] / 3 - 8, size[0] / 3 - 8} );
		// 	plots[i]->resizeEvent ( {size[0] / 3 - 8, size[0] / 3 - 8} );
		// }

		// performLayout();

		window->setPosition ( Eigen::Vector2i ( 5, 770 ) );
		graphs->setPosition ( Eigen::Vector2i ( size[0] - 220, 770 ) );
		texButtons->setPosition ( Eigen::Vector2i ( window->size() [0] + 20, 770 ) );
		texWindow->setPosition ( Eigen::Vector2i ( window->size() [0] + 20, 810 ) );
		texWindowRight->setPosition ( Eigen::Vector2i ( window->size() [0] + 300, 770 ) );

		// // needs to be called 2nd time
		// performLayout();
		// texView->fit();

		return true;

	}

	void makeWidgets() {

		nanogui::GridLayout *layout = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle,
		        15, 5 );
		layout->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill } );
		layout->setSpacing ( 0, 10 );

		nanogui::GridLayout *layout_2cols = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2,
		        nanogui::Alignment::Middle, 15, 5 );
		layout_2cols->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill } );
		layout_2cols->setSpacing ( 0, 10 );

		nanogui::GridLayout *layout_3cols = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 3,
		        nanogui::Alignment::Middle, 15, 5 );
		layout_2cols->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill } );
		layout_2cols->setSpacing ( 0, 10 );

		window = new nanogui::Window ( this, "" );
		window->setPosition ( Eigen::Vector2i ( 3, DEF_WIDTH / 3 + 3 ) );
		window->setLayout ( layout );

		Eigen::Vector2i bsize = Eigen::Vector2i ( 20, 20 );
		nanogui::Color bcolor = nanogui::Color ( 192, 120, 0, 45 );
		nanogui::Color ccolor = nanogui::Color ( 152, 80, 0, 45 );

		nanogui::Widget *window_params = new nanogui::Widget ( window );
		window_params->setLayout ( layout_3cols );
		{
			new nanogui::Label ( window_params, "Learning rate gen", "sans-bold" );
			nanogui::TextBox *textBox = new nanogui::TextBox ( window_params );
			textBox->setEditable ( true );
			textBox->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			textBox->setValue ( "1e-4" );
			textBox->setUnits ( "" );
			textBox->setDefaultValue ( "1e-4" );
			textBox->setFontSize ( 16 );
			textBox->setFormat ( "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?" );

			nanogui::Slider *slider = new nanogui::Slider ( window_params );
			slider->setValue ( 0.4f );
			slider->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			slider->setCallback ( [textBox] ( float value ) {
				float s = 1e-6 * pow ( 10, value * 5 );
				float lrate = s; std::cout << "learning rate g: " << lrate << std::endl;
				textBox->setValue ( string_format ( "%.*e", lrate ).c_str() );
				nn->learning_rate = lrate;

			} );

			new nanogui::Label ( window_params, "Learning rate d", "sans-bold" );
			textBox = new nanogui::TextBox ( window_params );
			textBox->setEditable ( true );
			textBox->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			textBox->setValue ( "1e-4" );
			textBox->setUnits ( "" );
			textBox->setDefaultValue ( "1e-4" );
			textBox->setFontSize ( 16 );
			textBox->setFormat ( "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?" );

			slider = new nanogui::Slider ( window_params );
			slider->setValue ( 0.4f );
			slider->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			slider->setCallback ( [textBox] ( float value ) {
				float s = 1e-6 * pow ( 10, value * 5 );
				float lrate = s; std::cout << "learning rate d: " << lrate << std::endl;
				textBox->setValue ( string_format ( "%.*e", lrate ).c_str() );
				discriminator->learning_rate = lrate;

			} );

			new nanogui::Label ( window_params, "Decay", "sans-bold" );
			nanogui::TextBox *textBox_decay = new nanogui::TextBox ( window_params );
			textBox_decay->setEditable ( true );
			textBox_decay->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			textBox_decay->setValue ( "0" );
			textBox_decay->setFontSize ( 16 );
			textBox_decay->setFormat ( "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?" );

			slider = new nanogui::Slider ( window_params );
			slider->setValue ( 0.0f );
			slider->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			slider->setCallback ( [textBox_decay] ( float value ) {
				float s = 1e-8 * pow ( 10, value * 5 ) - 1e-8;
				float decay = s; std::cout << "decay: " << decay << std::endl;
				textBox_decay->setValue ( string_format ( "%.*e", decay ).c_str() );
				nn->decay = decay;

			} );

			new nanogui::Label ( window_params, "beta", "sans-bold" );
			nanogui::TextBox *textBox_beta = new nanogui::TextBox ( window_params );
			textBox_beta->setEditable ( true );
			textBox_beta->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			textBox_beta->setValue ( "0" );
			textBox_beta->setFontSize ( 16 );
			textBox_beta->setFormat ( "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?" );

			slider = new nanogui::Slider ( window_params );
			slider->setValue ( 0.0f );
			slider->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			slider->setCallback ( [textBox_beta] ( float value ) {
				float s = 1e-7 * pow ( 10, value * 5 ) - 1e-7;
				float beta = s; std::cout << "beta: " << beta << std::endl;
				textBox_beta->setValue ( string_format ( "%.*e", beta ).c_str() );
				nn->set_sparsity_penalty ( beta );

			} );

			new nanogui::Label ( window_params, "ro", "sans-bold" );
			nanogui::TextBox *textBox_ro = new nanogui::TextBox ( window_params );
			textBox_ro->setEditable ( true );
			textBox_ro->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			textBox_ro->setValue ( "0.15" );
			textBox_ro->setFontSize ( 16 );
			textBox_ro->setFormat ( "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?" );

			slider = new nanogui::Slider ( window_params );
			slider->setValue ( 0.15f );
			slider->setFixedSize ( Eigen::Vector2i ( 100, 20 ) );
			slider->setCallback ( [textBox_ro] ( float value ) {
				float s = value;
				float ro = s; std::cout << "ro: " << ro << std::endl;
				textBox_ro->setValue ( string_format ( "%.*e", ro ).c_str() );
				nn->set_sparsity_target ( ro );

			} );

		}

		nanogui::Widget *opt_buttons_w;
		nanogui::Widget *opt_buttons;
		nanogui::Button *b;
		nanogui::Widget *control_buttons;

		plot_data->icons = nanogui::loadImageDirectory ( mNVGContext, "saved" );
		string resourcesFolderPath ( "./" );

		control_buttons = new nanogui::Widget ( window );
		control_buttons->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle,
		                             2, 2 ) );

		new nanogui::Widget ( control_buttons ); // empty
		new nanogui::Widget ( control_buttons ); // empty
		mProgress = new nanogui::ProgressBar ( control_buttons );
		new nanogui::Widget ( control_buttons ); // empty

		nanogui::PopupButton *imagePanelBtn = new nanogui::PopupButton ( control_buttons, "" );
		imagePanelBtn->setIcon ( ENTYPO_ICON_UPLOAD );
		imagePanelBtn->setFontSize ( 16 );
		imagePanelBtn->setFixedSize ( Eigen::Vector2i ( 80, 20 ) );
		imagePanelBtn->setBackgroundColor ( nanogui::Color ( 70, 70, 70, 65 ) );
		nanogui::Popup *popup = imagePanelBtn->popup();
		nanogui::VScrollPanel *vscroll = new nanogui::VScrollPanel ( popup );
		nanogui::ImagePanel *imgPanel = new nanogui::ImagePanel ( vscroll );
		imgPanel->setImages ( plot_data->icons );
		popup->setFixedSize ( Eigen::Vector2i ( 265, 160 ) );
		imagePanelBtn->setTooltip ( "load" );

		mCurrentImage = 0;

		imgPanel->setCallback ( [this, imgPanel] ( int i ) {

			// imageView->bindImage(mImagesData[i].first.texture());
			mCurrentImage = i;
			std::string fprefix = plot_data->icons[i].second;
			std::cout << "Loading " << fprefix +  + ".nn.bin" << '\n';
			nanogui::Serializer g ( std::string ( "g_" + fprefix + ".nn.bin" ).c_str(), false );
			nanogui::Serializer d ( std::string ( "d_" + fprefix + ".nn.bin" ).c_str(), false );
			nn->load ( g );
			discriminator->load ( d );
			std::cout << "Done." << '\n';
			// nn->testcode ( train_data, reconstruction_data );
			// plot_data->updated();
			// plot_data->p_vertices = nn->codes;

			// convert labels to colors, TODO: move somewhere else
			plot_data->p_colors.resize ( 3, nn->codes_colors.cols() );
			for ( int k = 0; k < nn->codes_colors.cols(); k++ ) {
				nanogui::Color c = nanogui::parula_lut[ ( int ) nn->codes_colors ( 0, k )];
				plot_data->p_colors.col ( k ) = Eigen::Vector3f ( c[0], c[1], c[2] );
			}


		} );


		imagePanelBtn = new nanogui::PopupButton ( control_buttons, "" );
		imagePanelBtn->setIcon ( ENTYPO_ICON_SAVE );
		imagePanelBtn->setFontSize ( 16 );
		imagePanelBtn->setFixedSize ( Eigen::Vector2i ( 80, 20 ) );
		imagePanelBtn->setBackgroundColor ( nanogui::Color ( 70, 70, 70, 65 ) );
		imagePanelBtn->setTooltip ( "save" );
		popup = imagePanelBtn->popup();

		nanogui::Widget *panel = new nanogui::Widget ( popup );
		panel->setLayout ( layout_2cols );

		nanogui::TextBox *textBox = new nanogui::TextBox ( panel );
		textBox->setEditable ( true );
		textBox->setValue ( "snapshot_" + return_current_time_and_date ( "%y%m%d_%H%M%S" ) );
		textBox->setFixedSize ( Eigen::Vector2i ( 245, 35 ) );
		textBox->setAlignment ( nanogui::TextBox::Alignment::Left );
		b = panel->add<nanogui::Button> ( "", ENTYPO_ICON_SAVE );
		b->setBackgroundColor ( nanogui::Color ( 192, 160, 0, 65 ) );

		b->setCallback ( [textBox, this] {

			std::string fprefix = textBox->value();
			std::cout << textBox->value() << std::endl;
			saveScreenShotCropped ( false, std::string ( "./saved/" + fprefix + ".png" ).c_str() );
			nanogui::Serializer g ( std::string ( "./saved/g_" + fprefix + ".nn.bin" ).c_str(), true );
			nanogui::Serializer d ( std::string ( "./saved/d_" + fprefix + ".nn.bin" ).c_str(), true );
			nn->save ( g );
			discriminator->save ( d );

		} ); b->setTooltip ( "save" );

		popup->setFixedSize ( Eigen::Vector2i ( 345, 60 ) );

		b = new nanogui::ToolButton ( control_buttons, ENTYPO_ICON_PLAY ); //25B6
		b->setFixedSize ( Eigen::Vector2i ( 80, 20 ) );
		b->setBackgroundColor ( nanogui::Color ( 192, 160, 0, 65 ) );
		b->setChangeCallback ( [this] ( bool pushed ) {

			nn->pause = !pushed;

		} ); b->setTooltip ( "pause/unpause" );

		b = control_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_CLOUD ); //25B6
		b->setFixedSize ( Eigen::Vector2i ( 80, 20 ) );
		b->setBackgroundColor ( nanogui::Color ( 0, 160, 190, 65 ) );
		b->setCallback ( [&]() {

			std::cout << "Generate start" << std::endl;
			gen_type dist = STRATIFIED;
			// gen_type { INDEPENDENT = 0, GRID = 1, STRATIFIED = 2} gen_type;

			size_t generate_point_count = 10000;
			generate ( std::normal_distribution<> ( 0, 0.02 ),
			           std::normal_distribution<> ( 0, 0.02 ),
			           std::normal_distribution<> ( 0, 0.02 ),
			           plot_data->s_vertices, generate_point_count, dist );

			std::cout << "Generate end" << std::endl;

			plot_data->updated();

		} ); b->setTooltip ( "generate" );

		texWindow = new nanogui::Window ( this, "" );
		texWindow->setPosition ( Eigen::Vector2i ( 410, 400 ) );
		texWindow->setLayout ( new nanogui::GroupLayout() );
		texWindow->setFixedSize ( {265, 265} );

		texWindowRight = new nanogui::Window ( this, "" );
		texWindowRight->setPosition ( Eigen::Vector2i ( 710, 400 ) );
		texWindowRight->setLayout ( new nanogui::GroupLayout() );
		texWindowRight->setFixedSize ( {325, 325} );

		opt_buttons_w = new nanogui::Widget ( window );
		opt_buttons_w->setLayout ( new nanogui::GroupLayout ( 0, 0, 0, 0 ) );
		opt_buttons = new nanogui::Widget ( opt_buttons_w );
		opt_buttons->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 15, nanogui::Alignment::Middle, 2,
		                         2 ) );

		b = opt_buttons->add<nanogui::Button> ( "SGD" ); b->setFontSize ( 8 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setPushed ( true );
		b->setCallback ( [&]() { std::cout << "SGD: " << std::endl; nn->otype = SGD;} );

		b = opt_buttons->add<nanogui::Button> ( "AD0" ); b->setFontSize ( 8 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( false );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "ADAGRAD: " << std::endl; nn->otype = ADAGRAD;} );

		b = opt_buttons->add<nanogui::Button> ( "AD1" ); b->setFontSize ( 8 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( false );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "ADADELTA: " << std::endl; nn->otype = ADADELTA;} );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_DOT );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "1: " << std::endl;} );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_TWO_DOTS );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "2: " << std::endl;} );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_THREE_DOTS );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "3: " << std::endl;} );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLOW_LINE );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "SEQ: " << std::endl; } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLOW_TREE );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "TREE: " << std::endl; } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLOW_BRANCH );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "BRANCH: " << std::endl; } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLOW_PARALLEL );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "PARALLEL: " << std::endl;} );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLOW_CASCADE );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "CASCADE: " << std::endl; } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_SHUFFLE );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "reset" << std::endl; nn->reset(); } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLASH );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "kick" << std::endl; nn->kick(); } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_FLASH );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "kick" << std::endl; nn->kick(); } );

		b = opt_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_BAR_GRAPH );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { nn->collect_stats_enabled = !nn->collect_stats_enabled; nnview->setVisible ( nn->collect_stats_enabled ); nnview->requestFocus();} );

		// Set the first texture

		// nanogui::ImagePanel *texPanel = new nanogui::ImagePanel(vscroll);
		// texPanel->setImages(icons);
		// mCurrentTex = 0;

		texView = new nanogui::ImageView ( texWindow, plot_data->input_data_textures.id );
		texViewRight = new nanogui::ImageView ( texWindowRight, plot_data->input_data_textures.id );

		// // Change the active textures.
		// texPanel->setCallback([this, imageView, texPanel](int i) {
		// 	imageView->bindImage(plot_data->input_data_textures.id);
		// 	mCurrentTex = i;
		// 	cout << "Selected texture " << i << '\n';
		// });

		texView->setGridThreshold ( 20 );
		texView->setPixelInfoThreshold ( 20 );
		texViewRight->setGridThreshold ( 20 );
		texViewRight->setPixelInfoThreshold ( 20 );

		texButtons = new nanogui::Window ( this, "" );
		texButtons->setLayout ( new nanogui::GroupLayout ( 0, 0, 0, 0 ) );
		nanogui::Widget *views = new nanogui::Widget ( texButtons );
		views->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 12, nanogui::Alignment::Middle, 2, 2 ) );

		int idx = 0;

		mCurrentTex = plot_data->input_data_textures.id;

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_DATABASE );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setPushed ( true );
		b->setCallback ( [&]() { std::cout << "T0: " << std::endl; mCurrentTex = plot_data->input_data_textures.id; texView->bindImage ( mCurrentTex ); } );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_LOGIN );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( false );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T1: " << std::endl; mCurrentTex = plot_data->nn_matrix_data_x[0].id; texView->bindImage ( mCurrentTex );} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_LOGOUT );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T2: " << std::endl; mCurrentTex = plot_data->nn_matrix_data_y.back().id; texView->bindImage ( mCurrentTex );} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CLOUD );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setPushed ( mCurrentTex == idx++ );
		b->setCallback ( [&]() { std::cout << "T3: " << std::endl; std::cout << plot_data->sample_reconstruction_textures.id << std::endl; mCurrentTex = plot_data->sample_reconstruction_textures.id; texView->bindImage ( mCurrentTex );} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_PROGRESS_0 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "mCurrentTex++; " << mCurrentTex + 1 << std::endl; mCurrentTex++; texView->bindImage ( mCurrentTex );} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_PROGRESS_1 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "mCurrentTex--; " << mCurrentTex - 1 << std::endl; mCurrentTex--; texView->bindImage ( mCurrentTex );} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_PROGRESS_2 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T6: " << std::endl; mCurrentTex = 6;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_PROGRESS_3 );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T7: " << std::endl; mCurrentTex = 7;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_LOCK );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T8: " << std::endl; mCurrentTex = 8;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_LOCK_OPEN );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T9: " << std::endl; mCurrentTex = 9;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_INFINITY );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T10: " << std::endl; mCurrentTex = 10;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_RIGHT_THIN );
		b->setFixedSize ( bsize ); b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "bind right button " << std::endl; texViewRight->bindImage ( mCurrentTex ); } );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CODE );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T12: " << std::endl; mCurrentTex = 12;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_THERMOMETER );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T13: " << std::endl; mCurrentTex = 13;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CYCLE );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T14: " << std::endl; mCurrentTex = 14;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CW );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T15: " << std::endl; mCurrentTex = 15;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CCW );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T16: " << std::endl; mCurrentTex = 16;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_MOON );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T17: " << std::endl; mCurrentTex = 17;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_BAR_GRAPH );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T18: " << std::endl; mCurrentTex = 18;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CAMERA );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T19: " << std::endl; mCurrentTex = 19;} );

		// ENTYPO_ICON_LIGHT_DOWN
		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_ADJUST );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T20: " << std::endl; mCurrentTex = 20;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_RECORD );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T21: " << std::endl; mCurrentTex = 21;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_STOP );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T22: " << std::endl; mCurrentTex = 22;} );

		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_BACK );
		b->setFlags ( nanogui::Button::RadioButton ); b->setFixedSize ( bsize ); b->setPushed ( mCurrentTex == idx++ );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "T23: " << std::endl; mCurrentTex = 23;} );

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_CASCADE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_BRANCH);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_TREE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_LINE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_PARALLEL);

		// texView->setPixelInfoCallback(
		// [this, texView](const Eigen::Vector2i & index) -> pair<string, nanogui::Color> {
		// 	auto& imageData = plot_data->input_data_textures.id;
		// 	auto& textureSize = imageView->imageSize();
		// 	std::string stringData;
		// 	uint16_t channelSum = 0;
		// 	for (int i = 0; i != 4; ++i) {
		// 		auto& channelData = imageData[4 * index.y() * textureSize.x() + 4 * index.x() + i];
		// 		channelSum += channelData;
		// 		stringData += (to_string(static_cast<int>(channelData)) + "\n");
		// 	}
		// 	float intensity = static_cast<float>(255 - (channelSum / 4)) / 255.0f;
		// 	float colorScale = intensity > 0.5f ? (intensity + 1) / 2 : intensity / 2;
		// 	nanogui::Color textColor = nanogui::Color(colorScale, 1.0f);
		// 	return { stringData, textColor };
		// });

		graphs = new nanogui::Window ( this, "" );
		graphs->setPosition ( Eigen::Vector2i ( window->size() [0] + 12, DEF_WIDTH / 3 + 5 ) );
		graphs->setLayout ( layout );

		graph_loss = new nanogui::Graph ( graphs, "" );
		graph_loss->setFooter ( "g loss" );
		graph_loss->setGraphColor ( nanogui::Color ( 128, 128, 128, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_loss->values().resize ( 500 );
		graph_loss->values().setZero();

		graph_fake_acc = new nanogui::Graph ( graphs, "" );
		graph_fake_acc->setFooter ( "d fake loss" );
		graph_fake_acc->setGraphColor ( nanogui::Color ( 128, 192, 0, 255 ) );
		graph_fake_acc->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 64 ) );
		graph_fake_acc->values().resize ( 500 );
		graph_fake_acc->values().setZero();

		graph_real_acc = new nanogui::Graph ( graphs, "" );
		graph_real_acc->setFooter ( "d real loss" );
		graph_real_acc->setGraphColor ( nanogui::Color ( 192, 64, 0, 255 ) );
		graph_real_acc->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 64 ) );
		graph_real_acc->values().resize ( 500 );
		graph_real_acc->values().setZero();

		graph_cpu = new nanogui::Graph ( graphs, "" );
		graph_cpu->setFooter ( "cpu ms per iter" );
		graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_cpu->values().resize ( 500 );
		graph_cpu->values().setZero();

		// FlOP/s
		graph_flops = new nanogui::Graph ( graphs, "" );
		graph_flops->setFooter ( "GFLOP / s" );
		graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_flops->values().resize ( 500 );
		graph_flops->values().setZero();

		// B/s
		graph_bytes = new nanogui::Graph ( graphs, "" );
		graph_bytes->setFooter ( "MB / s" );
		graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_bytes->values().resize ( 500 );
		graph_bytes->values().setZero();

		// /* FPS GRAPH */
		// graph_fps = new nanogui::Graph ( graphs, "" );
		// graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		// graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_fps->values().resize(500);
		// graph_fps->values().setZero();

		//legend
		nanogui::Graph *legend = new nanogui::Graph ( graphs, "", nanogui::GraphType::GRAPH_LEGEND );
		legend->values().resize ( 10 );
		legend->values() << 1, 1, 1, 1, 1, 1, 1, 1, 1, 1;

		large_image_view = new nanogui::Window ( this, "" );
		large_image_view->setFixedSize ( {670, 700} );

		large_image_view->setPosition ( {400, 25} );

		nanogui::Widget *large_w_buttons = new nanogui::Widget ( large_image_view );
		large_w_buttons->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle,
		                             2, 2 ) );

		b = large_w_buttons->add<nanogui::Button> ( "X" ); b->setFontSize ( 12 );
		b->setFixedSize ( {20, 20} );
		b->setBackgroundColor ( nanogui::Color ( 192, 0, 0, 65 ) );
		b->setCallback ( [this] {
			large_image_view->setVisible ( false );
		} ); b->setTooltip ( "close" );

		b = large_w_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_LAYOUT );
		b->setFixedSize ( {20, 20} );
		b->setBackgroundColor ( nanogui::Color ( 192, 160, 0, 65 ) );

		b->setCallback ( [this] {
			large_image_view->setVisible ( false );
		} ); b->setTooltip ( "close" );

		b = large_w_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_EXPORT );
		b->setFixedSize ( {20, 20} );
		b->setBackgroundColor ( nanogui::Color ( 192, 160, 0, 65 ) );

		b->setCallback ( [this] {
			large_image_view->setVisible ( false );
		} ); b->setTooltip ( "close" );

		b = large_w_buttons->add<nanogui::Button> ( "", ENTYPO_ICON_RESIZE_FULL );
		b->setFixedSize ( {20, 20} );
		b->setBackgroundColor ( nanogui::Color ( 192, 160, 0, 65 ) );
		b->setCallback ( [this] {
			large_image_view->setVisible ( false );
		} ); b->setTooltip ( "close" );

		large_image_view->setVisible ( false );
		large_tex_view = new nanogui::ImageView ( large_image_view, plot_data->input_data_textures.id );
		large_tex_view->setFixedSize ( {660, 660} );
		large_tex_view->setPosition ( {5, 35} );
		large_w_buttons->setPosition ( {5, 5} );
		large_w_buttons->setFixedSize ( {120, 30} );

		/* widgets end */


	}


	~GUI() { /* free resources */ }

	std::vector<Plot *> plots;
	PlotData *plot_data;
	nanogui::Window *root;
	nanogui::Window *window;
	nanogui::Window *large_image_view;
	nanogui::Window *graphs;
	nanogui::Window *controls, *texWindow, *texButtons, *texWindowRight;
	nanogui::Graph *graph_loss, *graph_fake_acc, *graph_real_acc;
	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;
	nanogui::ImageView *texView, *texViewRight, *large_tex_view;
	nanogui::ProgressBar *mProgress;
	NNView *nnview = nullptr;

	int completed_frames = 0;

	size_t local_data_checksum = 0;

	using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
	imagesDataType mImagesData;
	int mCurrentImage, mCurrentTex;

};

#endif
