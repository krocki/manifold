#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/serializer/core.h>
#include <nanogui/serializer/opengl.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>

// helpers
#include <utils.h>
#include <colors.h>
//#include <gl/tex.h>

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glmanifold.h>
#include <gui/glplothelper.h>

#include <gl/tex.h>

#define DEF_WIDTH 1100
#define DEF_HEIGHT 720
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



class GLTexture {
public:
	using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
	GLTexture() = default;
	GLTexture(const std::string& textureName)
		: mTextureName(textureName), mTextureId(0) {}

	GLTexture(const std::string& textureName, GLint textureId)
		: mTextureName(textureName), mTextureId(textureId) {}

	GLTexture(const GLTexture& other) = delete;
	GLTexture(GLTexture&& other) noexcept
		: mTextureName(std::move(other.mTextureName)),
		  mTextureId(other.mTextureId) {
		other.mTextureId = 0;
	}
	GLTexture& operator=(const GLTexture& other) = delete;
	GLTexture& operator=(GLTexture&& other) noexcept {
		mTextureName = std::move(other.mTextureName);
		std::swap(mTextureId, other.mTextureId);
		return *this;
	}
	~GLTexture() noexcept {
		if (mTextureId)
			glDeleteTextures(1, &mTextureId);
	}

	GLuint texture() const { return mTextureId; }
	const std::string& textureName() const { return mTextureName; }

	/**
	*  Load a file in memory and create an OpenGL texture.
	*  Returns a handle type (an std::unique_ptr) to the loaded pixels.
	*/
	handleType load(const std::string& fileName) {
		if (mTextureId) {
			glDeleteTextures(1, &mTextureId);
			mTextureId = 0;
		}
		int force_channels = 0;
		int w, h, n;
		handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
		if (!textureData)
			throw std::invalid_argument("Could not load texture data from file " + fileName);
		glGenTextures(1, &mTextureId);
		glBindTexture(GL_TEXTURE_2D, mTextureId);
		GLint internalFormat;
		GLint format;
		switch (n) {
		case 1: internalFormat = GL_R8; format = GL_RED; break;
		case 2: internalFormat = GL_RG8; format = GL_RG; break;
		case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
		case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
		default: internalFormat = 0; format = 0; break;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return textureData;
	}

private:
	std::string mTextureName;
	GLuint mTextureId;
};

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

		plots.push_back(new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, true, true, mGLFWWindow, mNVGContext, 50.0f, { -22.2f, -21.8f, -22.0f}, { 0.63, 1.895f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot0", t.c_str())));
		plots.push_back(new Plot ( root, "top frustum", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 1, plot_data, false, false, mGLFWWindow, mNVGContext, 66.0f, {0.0f, 35.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, {box_size * 2, box_size * 2, box_size * 2}, false, 0, string_format ( "%s_plot_top", t.c_str())));
		plots.push_back(new Plot ( root, "front ortho", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, false, false, mGLFWWindow, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, true, 0, string_format ( "%s_plot_ortho", t.c_str())));

		int number_of_cameras = plots.size();

		plot_data->e_vertices.resize(3, 2 * number_of_cameras);
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

		window = new nanogui::Window(this, "Grid of small widgets");
		window->setPosition(Eigen::Vector2i(3, DEF_WIDTH / 3 + 3));

		nanogui::GridLayout *layout =
		    new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,
		                            nanogui::Alignment::Middle, 15, 5);
		layout->setColAlignment(
		{ nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		layout->setSpacing(0, 10);
		window->setLayout(layout);

		/* FP widget */ {
			new nanogui::Label(window, "Floating point :", "sans-bold");
			nanogui::TextBox *textBox = new nanogui::TextBox(window);
			textBox->setEditable(true);
			textBox->setFixedSize(Eigen::Vector2i(100, 20));
			textBox->setValue("50");
			textBox->setUnits("GiB");
			textBox->setDefaultValue("0.0");
			textBox->setFontSize(16);
			textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
		}

		/* Positive integer widget */ {
			new nanogui::Label(window, "Positive integer :", "sans-bold");
			auto intBox = new nanogui::IntBox<int>(window);
			intBox->setEditable(true);
			intBox->setFixedSize(Eigen::Vector2i(100, 20));
			intBox->setValue(50);
			intBox->setUnits("Mhz");
			intBox->setDefaultValue("0");
			intBox->setFontSize(16);
			intBox->setFormat("[1-9][0-9]*");
			intBox->setSpinnable(true);
			intBox->setMinValue(1);
			intBox->setValueIncrement(2);
		}

		/* Checkbox widget */ {
			new nanogui::Label(window, "Checkbox :", "sans-bold");

			nanogui::CheckBox* cb = new nanogui::CheckBox(window, "Check me");
			cb->setFontSize(16);
			cb->setChecked(true);
		}

		new nanogui::Label(window, "Combo box :", "sans-bold");
		nanogui::ComboBox *cobo =
		    new nanogui::ComboBox(window, { "Item 1", "Item 2", "Item 3" });
		cobo->setFontSize(16);
		cobo->setFixedSize(Eigen::Vector2i(100, 20));

		new nanogui::Label(window, "Color button :", "sans-bold");
		nanogui::PopupButton* popupBtn = new nanogui::PopupButton(window, "", 0);
		popupBtn->setBackgroundColor(nanogui::Color(255, 120, 0, 255));
		popupBtn->setFontSize(16);
		popupBtn->setFixedSize(Eigen::Vector2i(100, 20));
		nanogui::Popup *popup = popupBtn->popup();
		popup->setLayout(new nanogui::GroupLayout());

		std::vector<pair<int, std::string>> icons = nanogui::loadImageDirectory(mNVGContext, "snapshots");
		string resourcesFolderPath("./");

		new nanogui::Label(window, "Image panel & scroll panel", "sans-bold");
		nanogui::PopupButton *imagePanelBtn = new nanogui::PopupButton(window, "Image Panel");
		imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
		popup = imagePanelBtn->popup();
		nanogui::VScrollPanel *vscroll = new nanogui::VScrollPanel(popup);
		nanogui::ImagePanel *imgPanel = new nanogui::ImagePanel(vscroll);
		imgPanel->setImages(icons);
		popup->setFixedSize(Eigen::Vector2i(245, 150));

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

		/* widgets end */

		performLayout();

		resizeEvent ( {DEF_WIDTH, DEF_HEIGHT} );

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
				return true;

				// case GLFW_KEY_S:

				// 	std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
				// 	saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
				// 	return true;

			}

		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual void frame_completed_event() {

		int interval = 250;

		if (completed_frames % interval == 0) {
			std::string t = return_current_time_and_date("%y%m%d_%H%M%S");
			saveScreenShot(false, string_format ("./snapshots/screens/screen_%s_%08d.png", t.c_str(), completed_frames).c_str());
		}

		completed_frames++;

	};

	virtual bool resizeEvent ( const Eigen::Vector2i & size ) {

		// 8 because of VGroupLayout(5) spacing
		for (int i = 0; i < (int) plots.size(); i++) {
			plots[i]->setSize({size[0] / 3 - 8, size[0] / 3 - 8});
			plots[i]->resizeEvent({size[0] / 3 - 8, size[0] / 3 - 8});
		}

		performLayout();

		window->setPosition(Eigen::Vector2i(3, size[0] / 3 + 3));
		// plot_helper->setPosition({0, size[0] / 3 + 10});
		// plot_helper->setFixedSize({root->size()[0] - 1, size[1] - root->size()[1] - 1});
		// plot_helper->setPosition({1, root->size()[1]});

		// needs to be called 2nd time
		performLayout();

		return true;

	}

	~GUI() { /* free resources */ }

	std::vector<Plot*> plots;
	PlotData *plot_data;
	nanogui::Window *root;
	nanogui::Window *window;

	int completed_frames = 0;

	size_t local_data_checksum = 0;

	using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
	imagesDataType mImagesData;
	int mCurrentImage;

};

#endif
