#ifndef __MANIFOLD_SCREEN_H__
#define __MANIFOLD_SCREEN_H__

#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/serializer/core.h>
#include <nanogui/serializer/opengl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// helpers
#include <utils.h>
#include <colors.h>

// app-specific GUI code
#include <gui/gldata.h>
#include <gui/glmanifold.h>
#include <gui/glplothelper.h>

#define DEF_WIDTH 1300
#define DEF_HEIGHT 800
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

		plots.push_back(new Plot ( root, "forward", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 0, plot_data, false, mGLFWWindow, mNVGContext, 50.0f, { -22.2f, -21.8f, -22.0f}, { 0.63, 1.895f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}));
		plots.push_back(new Plot ( root, "top frustum", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 1, plot_data, false, nullptr, mNVGContext, 40.0f, {0.0f, 30.0f, 0.0f}, {0.0f, 0.0f, -M_PI / 4.0f}, {box_size * 2, box_size * 2, box_size * 2}));
		plots.push_back(new Plot ( root, "front ortho", {DEF_WIDTH / 3, DEF_WIDTH / 3}, 2, plot_data, false, nullptr, mNVGContext, 40.0f, {0.0f, 0.0f, 11.0f}, {0.0f, 0.0f, 0.0f}, {box_size * 2, box_size * 2, box_size * 2}, true));
		plot_helper = new PlotHelper ( this, "" );
		plot_helper->setLayout ( new nanogui::GroupLayout ( 15, 0, 0, 0 ) );

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

	static int mini(int a, int b) { return a < b ? a : b; }

	static void unpremultiplyAlpha(unsigned char* image, int w, int h, int stride) {
		int x, y;

		// Unpremultiply
		for (y = 0; y < h; y++) {
			unsigned char *row = &image[y * stride];
			for (x = 0; x < w; x++) {
				int r = row[0], g = row[1], b = row[2], a = row[3];
				if (a != 0) {
					row[0] = (int)mini(r * 255 / a, 255);
					row[1] = (int)mini(g * 255 / a, 255);
					row[2] = (int)mini(b * 255 / a, 255);
				}
				row += 4;
			}
		}

		// Defringe
		for (y = 0; y < h; y++) {
			unsigned char *row = &image[y * stride];
			for (x = 0; x < w; x++) {
				int r = 0, g = 0, b = 0, a = row[3], n = 0;
				if (a == 0) {
					if (x - 1 > 0 && row[-1] != 0) {
						r += row[-4];
						g += row[-3];
						b += row[-2];
						n++;
					}
					if (x + 1 < w && row[7] != 0) {
						r += row[4];
						g += row[5];
						b += row[6];
						n++;
					}
					if (y - 1 > 0 && row[-stride + 3] != 0) {
						r += row[-stride];
						g += row[-stride + 1];
						b += row[-stride + 2];
						n++;
					}
					if (y + 1 < h && row[stride + 3] != 0) {
						r += row[stride];
						g += row[stride + 1];
						b += row[stride + 2];
						n++;
					}
					if (n > 0) {
						row[0] = r / n;
						row[1] = g / n;
						row[2] = b / n;
					}
				}
				row += 4;
			}
		}
	}

	static void setAlpha(unsigned char* image, int w, int h, int stride, unsigned char a) {
		int x, y;
		for (y = 0; y < h; y++) {
			unsigned char* row = &image[y * stride];
			for (x = 0; x < w; x++)
				row[x * 4 + 3] = a;
		}
	}

	static void flipHorizontal(unsigned char* image, int w, int h, int stride) {
		int i = 0, j = h - 1, k;
		while (i < j) {
			unsigned char* ri = &image[i * stride];
			unsigned char* rj = &image[j * stride];
			for (k = 0; k < w * 4; k++) {
				unsigned char t = ri[k];
				ri[k] = rj[k];
				rj[k] = t;
			}
			i++;
			j--;
		}
	}

	void saveScreenShot(int premult, const char* name) {

		int w = mFBSize[0];
		int h = mFBSize[1];

		unsigned char* image = (unsigned char*)malloc(w * h * 4);
		if (image == NULL)
			return;
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
		if (premult)
			unpremultiplyAlpha(image, w, h, w * 4);
		else
			setAlpha(image, w, h, w * 4, 255);
		flipHorizontal(image, w, h, w * 4);
		stbi_write_png(name, w, h, 4, image, w * 4);
		free(image);
	}

	virtual void drawContents() { }

	/* event handlers */
	virtual bool keyboardEvent ( int key, int scancode, int action, int modifiers ) {

		//todo: logf(type, "...", );

		printf ( "keyboardEvent: key %i scancode %i (%s), action %i mods %i\n", key, scancode, glfwGetKeyName ( key, scancode ), action, modifiers );

		if ( action ) {

			switch ( key ) {

			case GLFW_KEY_ESCAPE:

				saveScreenShot(false, "dump.png");
				setVisible ( false );
				return true;

			}
		}

		return Screen::keyboardEvent ( key, scancode, action, modifiers );
	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		// 8 because of VGroupLayout(5) spacing
		for (int i = 0; i < (int) plots.size(); i++) {
			plots[i]->setSize({size[0] / 3 - 8, size[0] / 3 - 8});
		}

		performLayout();

		plot_helper->setFixedSize({root->size()[0] - 1, size[1] - root->size()[1] - 1});
		plot_helper->setPosition({1, root->size()[1]});

		// needs to be called 2nd time
		performLayout();

		return true;

	}

	~GUI() { /* free resources */ }

	std::vector<Plot*> plots;
	PlotData *plot_data;
	PlotHelper *plot_helper;
	nanogui::Window *root;

};

#endif
