#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/serializer/core.h>
#include <nanogui/serializer/opengl.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/entypo.h>

// helpers
#include <utils.h>
#include <colors.h>

#include "aux.h"
#include "perf.h"
#include "fps.h"

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glmanifold.h>
#include <gui/glplothelper.h>
#include <compute/functions.h>

#include <gl/tex.h>

#define DEF_WIDTH 1300
#define DEF_HEIGHT 770
#define SCREEN_NAME "Manifold"
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

		nanogui::Screen ( { DEF_WIDTH , DEF_HEIGHT  }, SCREEN_NAME, RESIZABLE, FULLSCREEN, COLOR_BITS, ALPHA_BITS, DEPTH_BITS, STENCIL_BITS, MSAA_SAMPLES, GL_MAJOR, GL_MINOR, VSYNC, AUTOSIZE, SIZE_RATIO) { init(); }

	void init() {

		// get GL capabilities info
		printf ( "GL_RENDERER: %s\n", glGetString ( GL_RENDERER ) );
		printf ( "GL_VERSION: %s\n", glGetString ( GL_VERSION ) );
		printf ( "GLSL_VERSION: %s\n\n", glGetString ( GL_SHADING_LANGUAGE_VERSION ) );

		// init data
		plot_data = new PlotData();

		float box_size = 10.0f;

		generate_cube(plot_data->c_indices, plot_data->c_vertices, plot_data->c_colors, {0, 0, 0}, box_size, {0.5f, 0.5f, 0.5f});

		plot_data->updated();

		// gui elements
		root = new nanogui::Window ( this, "" );
		root->setLayout ( new nanogui::VGroupLayout(5) );

		std::string t = return_current_time_and_date("%y%m%d_%H%M%S");

		plots.push_back(new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, true, true, true, false, mGLFWWindow, mNVGContext, 80.0f, { -15.2f, -15.8f, -15.0f}, { 0.63, 1.895f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot0", t.c_str())));
		// plots.push_back(new Plot ( root, "front", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, true, true, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot_ortho", t.c_str())));

		plots.push_back(new Plot ( root, "top frustum", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 1, plot_data, true, false, false, true, mGLFWWindow, mNVGContext, 66.0f, {0.0f, 35.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot_top", t.c_str())));
		plots.push_back(new Plot ( root, "front ortho", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, false, false, false, false, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, true, 0, string_format ( "%s_plot_ortho", t.c_str())));

		int number_of_cameras = plots.size();

		plot_data->e_vertices.resize(3, 2 * number_of_cameras);
		plot_data->r_vertices.resize(3, 2 * number_of_cameras);
		plot_data->e_colors.resize(3, 2 * number_of_cameras);

		plot_data->e_colors.col(0) << 1, 0, 0; plot_data->e_colors.col(1) << 1, 0, 0;
		plot_data->e_colors.col(2) << 0, 1, 0; plot_data->e_colors.col(3) << 0, 1, 0;
		plot_data->e_colors.col(4) << 0, 0, 1; plot_data->e_colors.col(5) << 0, 0, 1;

		// share shader data
		for (int i = 0; i < number_of_cameras; i++) {
			plots[i]->master_pointShader = plots[0]->m_pointShader;
		}

		// todo: set/save layout (including dynamically created widgets)
		// be able to load everything

		drawAll();
		setVisible ( true );

		/* widgets */
		makeWidgets();

		performLayout();

		resizeEvent ( {DEF_WIDTH, DEF_HEIGHT} );

		// DATA
		train_data = MNISTImporter::importFromFile ( "data/mnist/train-images-idx3-ubyte", "data/mnist/train-labels-idx1-ubyte" );
		test_data = MNISTImporter::importFromFile ( "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte" );

		update_data_textures();

	}

	void save(nanogui::Serializer &s) const {

		s.set("position", mPos);
		s.set("size", mSize);
		s.set("fixedSize", mFixedSize);
		s.set("visible", mVisible);
		s.set("enabled", mEnabled);
		s.set("focused", mFocused);
		s.set("tooltip", mTooltip);
		s.set("fontSize", mFontSize);
		s.set("cursor", (int) mCursor);
	}

	void saveScreenShot(int premult, const char* name) {

		int w = mFBSize[0];
		int h = mFBSize[1];

		screenshot(0, 0, w, h, premult, name);

	}

	void saveScreenShotCropped(int premult, const char* name) {

		int w = mFBSize[0];
		int h = mFBSize[1];
		int x = 0;
		int y = h - w / 3;
		h = w / 3;
		w = w / 3;

		screenshot(x, y, w, h, premult, name);

	}

	virtual void drawContents() { }

	/* event handlers */
	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		//todo: logf(type, "...", );

		printf ( "keyboardEvent: key %i scancode %i (%s), action %i mods %i\n", key, scancode, glfwGetKeyName ( key, scancode ), action, modifiers );

		if ( action ) {

			switch ( key ) {

			case GLFW_KEY_ESCAPE:

				saveScreenShotCropped(false, "dump.png");
				setVisible ( false );
				nn->pause = false;
				nn->quit = true;
				return true;

			case GLFW_KEY_F12:

				std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
				saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
				return true;

			}

		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual void frame_completed_event() {

		int interval = 250;

		if (completed_frames % interval == 0) {
			std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
			// saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
		}

		completed_frames++;

		if (nn)
			if ( nn->clock ) {
				nn->clock = false;
				update_graph ( graph_loss, nn->current_loss );
				update_graph ( graph_cpu, cpu_util, 1000.0f, "ms" );
				update_graph ( graph_flops, cpu_flops, 1.0f, "GF/s" );
				update_graph ( graph_bytes, cpu_reads, 1.0f, "MB/s" );
			}

	};

	void update_reconstructions() {

		plot_data->update_reconstruction_textures(mNVGContext);

	}

	void update_data_textures() {

		plot_data->load_data_textures(train_data, mNVGContext);

	}

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		// 8 because of VGroupLayout(5) spacing
		for (int i = 0; i < (int) plots.size(); i++) {
			plots[i]->setSize({size[0] / 3 - 8, size[0] / 3 - 8});
			plots[i]->resizeEvent({size[0] / 3 - 8, size[0] / 3 - 8});
		}

		performLayout();

		window->setPosition(Eigen::Vector2i(5, size[0] / 3 + 8));
		graphs->setPosition(Eigen::Vector2i(window->size()[0] + 12, size[0] / 3 + 8));
		norm_graphs->setPosition(Eigen::Vector2i(window->size()[0] + 232, size[0] / 3 + 8));
		// controls->setPosition(Eigen::Vector2i(window->size()[0] + graphs->size()[0] + 8, size[0] / 3 + 5));
		// needs to be called 2nd time
		performLayout();

		return true;

	}

	void makeWidgets() {

		nanogui::GridLayout *layout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 15, 5);
		layout->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout->setSpacing(0, 10);

		nanogui::GridLayout *layout_2cols = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 15, 5);
		layout_2cols->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout_2cols->setSpacing(0, 10);

		nanogui::GridLayout *layout_3cols = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 15, 5);
		layout_2cols->setColAlignment( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout_2cols->setSpacing(0, 10);


		// nanogui::Label* message = new nanogui::Label(window, "", "sans-bold");

		window = new nanogui::Window(this, "");
		window->setPosition(Eigen::Vector2i(3, DEF_WIDTH / 3 + 3));
		window->setLayout(layout);
		// /* Positive integer widget */ {
		// 	new nanogui::Label(window, "Positive integer :", "sans-bold");
		// 	auto intBox = new nanogui::IntBox<int>(window);
		// 	intBox->setEditable(true);
		// 	intBox->setFixedSize(Eigen::Vector2i(100, 20));
		// 	intBox->setValue(50);
		// 	intBox->setUnits("Mhz");
		// 	intBox->setDefaultValue("0");
		// 	intBox->setFontSize(16);
		// 	intBox->setFormat("[1-9][0-9]*");
		// 	intBox->setSpinnable(true);
		// 	intBox->setMinValue(1);
		// 	intBox->setValueIncrement(2);
		// }

		// /* Checkbox widget */ {
		// 	new nanogui::Label(window, "Checkbox :", "sans-bold");

		// 	nanogui::CheckBox* cb = new nanogui::CheckBox(window, "Check me");
		// 	cb->setFontSize(16);
		// 	cb->setChecked(true);

		// }

		// new nanogui::Label(window, "Combo box :", "sans-bold");
		// nanogui::ComboBox *cobo =
		//     new nanogui::ComboBox(window, { "Item 1", "Item 2", "Item 3" });
		// cobo->setFontSize(16);
		// cobo->setFixedSize(Eigen::Vector2i(100, 20));

		// new nanogui::Label(window, "Color button :", "sans-bold");
		// nanogui::PopupButton* popupBtn = new nanogui::PopupButton(window, "", 0);
		// popupBtn->setBackgroundColor(nanogui::Color(255, 120, 0, 255));
		// popupBtn->setFontSize(16);
		// popupBtn->setFixedSize(Eigen::Vector2i(100, 20));
		// nanogui::Popup *popup = popupBtn->popup();
		// popup->setLayout(new nanogui::GroupLayout());

		std::vector<pair<int, std::string>> icons = nanogui::loadImageDirectory(mNVGContext, "saved");
		string resourcesFolderPath("./");


		nanogui::PopupButton *imagePanelBtn = new nanogui::PopupButton(window, "Load");
		imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
		nanogui::Popup *popup = imagePanelBtn->popup();
		nanogui::VScrollPanel *vscroll = new nanogui::VScrollPanel(popup);
		nanogui::ImagePanel *imgPanel = new nanogui::ImagePanel(vscroll);
		imgPanel->setImages(icons);
		popup->setFixedSize(Eigen::Vector2i(265, 160));

		mCurrentImage = 0;

		// auto imageWindow = new nanogui::Window(this, "Selected image");
		// auto imageView = new nanogui::ImageView(imageWindow, mImagesData[0].first.texture());

		imgPanel->setCallback([this, icons, imgPanel](int i) {
			// imageView->bindImage(mImagesData[i].first.texture());
			mCurrentImage = i;
			std::string fprefix = icons[i].second;
			std::cout << "Loading " << fprefix +  + ".nn.bin" << '\n';
			nanogui::Serializer s(std::string(fprefix + ".nn.bin").c_str(), false);
			nn->load(s);
			std::cout << "Done." << '\n';
			nn->testcode ( train_data );
			std::cout << nn->codes << std::endl;
			nn->generate(nn->codes, plot_data->p_reconstructions);
			update_reconstructions();
			plot_data->updated();
			plot_data->p_vertices = nn->codes;

			// convert labels to colors, TODO: move somewhere else
			plot_data->p_colors.resize(3, nn->codes_colors.cols());
			for (int k = 0; k < nn->codes_colors.cols(); k++) {
				nanogui::Color c = nanogui::parula_lut[(int)nn->codes_colors(0, k)];
				plot_data->p_colors.col(k) = Eigen::Vector3f(c[0], c[1], c[2]);
			}


		});

		imagePanelBtn = new nanogui::PopupButton(window, "Save");
		imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
		popup = imagePanelBtn->popup();

		nanogui::Widget *panel = new nanogui::Widget(popup);
		panel->setLayout(layout_2cols);

		nanogui::TextBox* textBox = new nanogui::TextBox(panel);
		textBox->setEditable(true);
		textBox->setValue("snapshot_" + return_current_time_and_date("%y%m%d_%H%M%S"));
		textBox->setFixedSize(Eigen::Vector2i(245, 35));
		textBox->setAlignment(nanogui::TextBox::Alignment::Left);
		nanogui::Button* b = panel->add<nanogui::Button>("", ENTYPO_ICON_SAVE);
		b->setBackgroundColor(nanogui::Color(192, 160, 0, 65));

		b->setCallback([textBox, this] {

			std::string fprefix = textBox->value();
			std::cout << textBox->value() << std::endl;
			saveScreenShotCropped(false, std::string("./saved/" + fprefix + ".png").c_str());
			nanogui::Serializer s(std::string("./saved/" + fprefix + ".nn.bin").c_str(), true);
			nn->save(s);

		}); b->setTooltip("save");

		popup->setFixedSize(Eigen::Vector2i(345, 60));

		// controls = new nanogui::Window(window, "");
		// controls->setPosition(Eigen::Vector2i(window->size()[0] + graphs->size()[0] + 8, DEF_WIDTH / 3 + 5));
		// controls->setLayout(layout);

		b = new nanogui::ToolButton(window, ENTYPO_ICON_PLAY); //25B6
		b->setFixedSize(Eigen::Vector2i(90, 20));
		b->setBackgroundColor(nanogui::Color(192, 160, 0, 65));
		b->setChangeCallback([this](bool pushed) {

			nn->pause = !pushed;

		}); b->setTooltip("pause/unpause");


		b = window->add<nanogui::Button>("", ENTYPO_ICON_CLOUD); //25B6
		b->setFixedSize(Eigen::Vector2i(90, 20));
		b->setBackgroundColor(nanogui::Color(192, 160, 0, 65));
		b->setCallback([this]() {

			size_t point_count = 50000;
			generate ( std::normal_distribution<> ( 0, 0.35 ),
			           std::normal_distribution<> ( 0, 0.35 ),
			           std::normal_distribution<> ( 0, 0.35 ),
			           plot_data->p_vertices, point_count, STRATIFIED );
			nn->generate ( plot_data->p_vertices, plot_data->p_reconstructions );
			func3::set ( {0.0f, 1.0f, 0.0f}, plot_data->p_colors, point_count );

			update_reconstructions();
			plot_data->updated();

		}); b->setTooltip("reconstructions");

		nanogui::Window* window_params = new nanogui::Window(window, "");
		window_params->setLayout(layout);
		{
			new nanogui::Label(window_params, "Learning rate", "sans-bold");
			nanogui::TextBox *textBox = new nanogui::TextBox(window_params);
			textBox->setEditable(true);
			textBox->setFixedSize(Eigen::Vector2i(100, 20));
			textBox->setValue("1e-4");
			textBox->setUnits("");
			textBox->setDefaultValue("1e-4");
			textBox->setFontSize(16);
			textBox->setFormat("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

			nanogui::Slider* slider = new nanogui::Slider(window_params);
			slider->setValue(0.4f);
			slider->setFixedSize(Eigen::Vector2i(100, 20));
			slider->setCallback([textBox](float value) {
				float s = 1e-6 * pow(10, value * 5);
				float lrate = s; std::cout << "learning rate: " << lrate << std::endl;
				textBox->setValue(string_format("%.*e", lrate).c_str());
				nn->learning_rate = lrate;

			});

			new nanogui::Label(window_params, "Decay", "sans-bold");
			nanogui::TextBox *textBox_decay = new nanogui::TextBox(window_params);
			textBox_decay->setEditable(true);
			textBox_decay->setFixedSize(Eigen::Vector2i(100, 20));
			textBox_decay->setValue("0");
			textBox_decay->setFontSize(16);
			textBox_decay->setFormat("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

			slider = new nanogui::Slider(window_params);
			slider->setValue(0.0f);
			slider->setFixedSize(Eigen::Vector2i(100, 20));
			slider->setCallback([textBox_decay](float value) {
				float s = 1e-8 * pow(10, value * 5) - 1e-8;
				float decay = s; std::cout << "decay: " << decay << std::endl;
				textBox_decay->setValue(string_format("%.*e", decay).c_str());
				nn->decay = decay;

			});

		}

		// auto imageWindow = new nanogui::Window(this, "Selected image");
		// imageWindow->setPosition(Eigen::Vector2i(410, 400));
		// imageWindow->setLayout(new nanogui::GroupLayout());

		// // Load all of the images by creating a GLTexture object and saving the pixel data.
		// for (auto& icon : icons) {
		// 	GLTexture texture(icon.second);
		// 	auto data = texture.load(resourcesFolderPath + icon.second + ".png");
		// 	mImagesData.emplace_back(std::move(texture), std::move(data));
		// }

		// // Set the first texture
		// auto imageView = new nanogui::ImageView(imageWindow, mImagesData[0].first.texture());
		// mCurrentImage = 0;
		// // Change the active textures.
		// imgPanel->setCallback([this, imageView, imgPanel](int i) {
		// 	imageView->bindImage(mImagesData[i].first.texture());
		// 	mCurrentImage = i;
		// 	cout << "Selected item " << i << '\n';
		// });
		// imageView->setGridThreshold(20);
		// imageView->setPixelInfoThreshold(20);
		// imageView->setPixelInfoCallback(
		// [this, imageView](const Eigen::Vector2i & index) -> pair<string, nanogui::Color> {
		// 	auto& imageData = mImagesData[mCurrentImage].second;
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
		graphs = new nanogui::Window(this, "");
		graphs->setPosition(Eigen::Vector2i(window->size()[0] + 12, DEF_WIDTH / 3 + 5));
		graphs->setLayout(layout);

		graph_loss = new nanogui::Graph ( graphs, "" );
		graph_loss->setFooter ( "loss" );
		graph_loss->setGraphColor ( nanogui::Color ( 128, 128, 128, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_loss->values().resize(500);
		graph_loss->values().setZero();

		graph_cpu = new nanogui::Graph ( graphs, "" );
		graph_cpu->setFooter ( "cpu ms per iter" );
		graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_cpu->values().resize(500);
		graph_cpu->values().setZero();

		// FlOP/s
		graph_flops = new nanogui::Graph ( graphs, "" );
		graph_flops->setFooter ( "GFLOP/s" );
		graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_flops->values().resize(500);
		graph_flops->values().setZero();

		// B/s
		graph_bytes = new nanogui::Graph ( graphs, "" );
		graph_bytes->setFooter ( "MB/s" );
		graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_bytes->values().resize(500);
		graph_bytes->values().setZero();

		norm_graphs = new nanogui::Window(this, "");
		norm_graphs->setPosition(Eigen::Vector2i(window->size()[0] + 230, DEF_WIDTH / 3 + 5));
		norm_graphs->setLayout(layout);

		for (size_t i = 0; i < 7; i++) {

			// graph_norms.push_back(new nanogui::Graph ( norm_graphs, "" ));
			// graph_norms.back()->setFooter ( string_format("%d", i).c_str() );
			// graph_norms.back()->setGraphColor ( nanogui::Color ( 255, 255, 150, 255 ) );
			// graph_norms.back()->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			// graph_norms.back()->setFixedSize({100, 30});

			graph_norms.push_back(new nanogui::Graph ( norm_graphs, "" ));
			graph_norms.back()->setFooter ( string_format("%d", i).c_str() );
			graph_norms.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 255 ) );
			graph_norms.back()->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			graph_norms.back()->setFixedSize({100, 30});

			// graph_norms.push_back(new nanogui::Graph ( norm_graphs, "" ));
			// graph_norms.back()->setFooter ( string_format("%d", i).c_str() );
			// graph_norms.back()->setGraphColor ( nanogui::Color ( 150, 255, 255, 255 ) );
			// graph_norms.back()->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
			// graph_norms.back()->setFixedSize({100, 30});

		}

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

		/* widgets end */


	}

	~GUI() { /* free resources */ }

	std::vector<Plot*> plots;
	PlotData *plot_data;
	nanogui::Window *root;
	nanogui::Window *window;
	nanogui::Window *graphs, *norm_graphs;
	nanogui::Window *controls;
	nanogui::Graph *graph_loss;
	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;
	std::vector<nanogui::Graph *> graph_norms;

	int completed_frames = 0;

	size_t local_data_checksum = 0;

	using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
	imagesDataType mImagesData;
	int mCurrentImage;

};

#endif
