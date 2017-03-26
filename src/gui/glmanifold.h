/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:09:39
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-26 00:17:08
*/

#ifndef __GLPLOT_H__
#define __GLPLOT_H__

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/glcanvas.h>
#include <nanogui/label.h>
#include <nanogui/entypo.h>
#include <nanogui/toolbutton.h>
#include <nanogui/slider.h>

#include <gui/gldata.h>
// nvgCreateImageA
#include <gl/tex.h>

#define POINT_SHADER_NAME "datapoint_shader"
#define POINT_FRAG_FILE "./src/glsl/datapoint.f.glsl"
#define POINT_VERT_FILE "./src/glsl/datapoint.v.glsl"

#define CAM_SHADER_NAME "cam_shader"
#define CAM_FRAG_FILE "./src/glsl/cam.f.glsl"
#define CAM_GEOM_FILE "./src/glsl/cam.g.glsl"
#define CAM_VERT_FILE "./src/glsl/cam.v.glsl"

#define BOX_SHADER_NAME "box_shader"
#define BOX_FRAG_FILE "./src/glsl/surf_box.f.glsl"
#define BOX_GEOM_FILE "./src/glsl/surf_box.g.glsl"
#define BOX_VERT_FILE "./src/glsl/surf_box.v.glsl"

class Plot : public nanogui::GLCanvas {

public:

	Plot ( Widget *parent, std::string _caption, const Eigen::Vector2i &w_size, int i, PlotData *plot_data, bool transparent = false, bool _keyboard_enabled = false, GLFWwindow *w = nullptr, NVGcontext *nvg = nullptr, float _fovy = 67.0f, const Eigen::Vector3f _camera = Eigen::Vector3f(0.0f, 0.0f, 5.0f), const Eigen::Vector3f _rotation = Eigen::Vector3f(0.0f, 0.0f, 0.0f), const Eigen::Vector3f _box_size = Eigen::Vector3f(1.0f, 1.0f, 1.0f) , bool _ortho = false, int record_intvl = 0, const std::string r_prefix = "") : nanogui::GLCanvas ( parent, transparent ) {

		GLCanvas::setSize (w_size);
		glfw_window = w;
		keyboard_enabled = _keyboard_enabled;

		vg = nvg;

		data = nullptr;

		setBackgroundColor ( nanogui::Color ( 64, 64, 64, 64 ) );
		setDrawBorder ( true );
		setVisible ( true );

		forward = Eigen::Vector3f (0.0f, 0.0f, -1.0f);
		right = Eigen::Vector3f ( 1.0f, 0.0f, 0.0f);
		up = Eigen::Vector3f (0.0f, 1.0f, 0.0f);

		index = i;
		caption = _caption;
		bind_data(plot_data);

		translation.setZero();
		total_translation.setZero();
		total_rotation.setZero();

		init_camera(_fovy, _camera, _rotation, _ortho);
		box_size = _box_size;
		init_shaders();

		record_interval = record_intvl;
		record_prefix = r_prefix;
		tic = glfwGetTime();

		tools = new nanogui::Widget(this);
		tools->setLayout(new nanogui::GroupLayout(0, 0, 0, 0));

		nanogui::Widget *arrows = new nanogui::Widget(tools);
		arrows->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 2, 2));

		nanogui::Button *b;

		b = new nanogui::ToolButton(arrows, ENTYPO_ICON_TRIANGLE_LEFT); b->setCallback([&] { std::cout << "left" << std::endl; translation[0] -= cam_speed; }); b->setTooltip("left");
		b = new nanogui::ToolButton(arrows, ENTYPO_ICON_TRIANGLE_DOWN); b->setCallback([&] { std::cout << "down" << std::endl; translation[1] -= cam_speed; }); b->setTooltip("down");
		b = new nanogui::ToolButton(arrows, ENTYPO_ICON_TRIANGLE_UP); b->setCallback([&] { std::cout << "up" << std::endl; translation[1] += cam_speed; }); b->setTooltip("up");
		b = new nanogui::ToolButton(arrows, ENTYPO_ICON_TRIANGLE_RIGHT); b->setCallback([&] { std::cout << "right" << std::endl; translation[0] += cam_speed; }); b->setTooltip("right");

		nanogui::Slider *slider = new nanogui::Slider(tools);
		slider->setValue(0.5f);
		slider->setFixedWidth(80);
		slider->setCallback([&](float value) {

			fovy = (value * 100.0f);
			std::cout << "fovy: " << fovy << std::endl;

		});

		slider = new nanogui::Slider(tools);
		slider->setValue(cam_speed);
		slider->setFixedWidth(80);
		slider->setCallback([&](float value) {
			cam_speed = value; std::cout << "cam speed: " << cam_speed << std::endl;

		});

		tools->setPosition({w_size[0] - 110, 0});

	}

	void init_shaders() {

		m_pointShader = new nanogui::GLShader();
		m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		m_cubeShader = new nanogui::GLShader();
		m_cubeShader->initFromFiles ( BOX_SHADER_NAME, BOX_VERT_FILE, BOX_FRAG_FILE, BOX_GEOM_FILE );

		size_t tex_size = 64;
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image = Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>::Ones(tex_size, tex_size * 4) * 16;
		// textures.emplace_back ( std::pair<int, std::string> ( nvgCreateImageA ( vg, 512, 512, NVG_IMAGE_GENERATE_MIPMAPS, ( unsigned char * ) rgba_image.data() ), "" ) );
		textures.emplace_back ( std::pair<int, std::string> ( nvgCreateImageRGBA ( vg,  tex_size, tex_size, NVG_IMAGE_NEAREST, ( unsigned char * ) rgba_image.data() ), "" ) );

		m_camShader = new nanogui::GLShader();
		m_camShader->initFromFiles ( CAM_SHADER_NAME, CAM_VERT_FILE, CAM_FRAG_FILE, CAM_GEOM_FILE );

	}

	void init_camera(float _fovy, const Eigen::Vector3f _camera,  const Eigen::Vector3f _rotation, bool _ortho = false) {

		ortho = _ortho;

		camera = _camera;
		rotation = _rotation;
		fovy = _fovy;

		q.setIdentity();
		q = rotate(rotation, forward, up, right) * q;

		near = 0.1f;
		far = 100.0f;

		cam_speed = 0.5f;
		cam_angular_speed = 0.0003f * 360.0f / M_PI;

	}

	void update_projection() {

		float fH;
		float fW;

		if (ortho) {

			fH = 11.0f;
			fW = 11.0f;
			near = 0.0f;
			far = fH + fW;
			proj = nanogui::ortho(-fW, fW, -fH, fH, near, far);

		} else {
			fH = std::tan(fovy / 360.0f * M_PI) * near;
			fW = fH * (float) mSize.x() / (float) mSize.y();
			proj = nanogui::frustum(-fW, fW, -fH, fH, near, far);
		}

	}

	void update_view() {

		q = rotate(rotation, forward, up, right) * q;
		R = quat_to_mat(q);

		forward = (R * (-1.0f * Eigen::Vector4f::UnitZ())).block(0, 0, 3, 1);
		right = (R * (Eigen::Vector4f::UnitX())).block(0, 0, 3, 1);
		up = (R * (Eigen::Vector4f::UnitY())).block(0, 0, 3, 1);

		camera += forward * (-translation[2]);
		camera += up * translation[1];
		camera += right * translation[0];

		total_rotation += rotation;
		total_translation += translation;

		rotation.setZero();
		translation.setZero();

		T = translate({camera[0], camera[1], camera[2]});
		view = R.inverse() * T.inverse();

	}

	void update_model() {

		model.setIdentity();
		data_model = translate({ -box_size[0] / 2, -box_size[1] / 2, -box_size[2] / 2});

	}

	void bind_data(PlotData* d) {

		data = d;

	}

	void unbind_data() {

		data = nullptr;

	}

	void refresh_data() {

		/* Upload points to GPU */

		if (data) {

			// if (index == 0) {

			m_pointShader->bind();
			m_pointShader->uploadAttrib("position", data->p_vertices);
			m_pointShader->uploadAttrib("color", data->p_colors);

			// }

			m_cubeShader->bind();
			m_cubeShader->uploadIndices ( data->c_indices );
			m_cubeShader->uploadAttrib("position", data->c_vertices);
			m_cubeShader->uploadAttrib("color", data->c_colors);

			local_data_checksum = data->checksum;

		} else {

			/* printf("data is null... not refreshing\n") */;

		}

	}

	void refresh_camera_positions() {

		// update cam position
		data->e_vertices.col (2 * index) = camera - forward;
		data->e_vertices.col (2 * index + 1) = camera;

		m_camShader->bind();
		m_camShader->uploadAttrib("position", data->e_vertices);
		m_camShader->uploadAttrib("color", data->e_colors);

	}


	void process_keyboard() {

		if (glfw_window && keyboard_enabled) {

			if ( glfwGetKey ( glfw_window, 'A' ) == GLFW_PRESS ) translation[0] -= cam_speed;
			if ( glfwGetKey ( glfw_window, 'D' ) == GLFW_PRESS ) translation[0] += cam_speed;
			if ( glfwGetKey ( glfw_window, 'E' ) == GLFW_PRESS ) translation[1] -= cam_speed;
			if ( glfwGetKey ( glfw_window, 'Q' ) == GLFW_PRESS ) translation[1] += cam_speed;
			if ( glfwGetKey ( glfw_window, 'W' ) == GLFW_PRESS ) translation[2] -= cam_speed;
			if ( glfwGetKey ( glfw_window, 'S' ) == GLFW_PRESS ) translation[2] += cam_speed;

			if ( glfwGetKey ( glfw_window, 'Z' ) == GLFW_PRESS ) rotation[0] -= cam_angular_speed;
			if ( glfwGetKey ( glfw_window, 'C' ) == GLFW_PRESS ) rotation[0] += cam_angular_speed;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_RIGHT ) == GLFW_PRESS ) rotation[1] -= cam_angular_speed;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_LEFT ) == GLFW_PRESS ) rotation[1] += cam_angular_speed;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_DOWN ) == GLFW_PRESS ) rotation[2] -= cam_angular_speed;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_UP ) == GLFW_PRESS ) rotation[2] += cam_angular_speed;

		}

	}

	void update_mvp() {

		// update model, view, projection

		update_model();
		update_view();
		update_projection();

		mvp = proj * view * model;
		data_mvp = proj * view * data_model;

	}

	void update_frame_count() {

		frame_time = glfwGetTime() - tic;
		num_frames++;

		tic = glfwGetTime();

	}

	void drawGL() override {

		update_frame_count();

		process_keyboard();

		/* Upload points to GPU */

		bool record = false;
		if (data->checksum != local_data_checksum) {
			refresh_data();
			record = true;
		}

		refresh_camera_positions();

		update_mvp();

		/* Render */

		glDisable ( GL_DEPTH_TEST );
		//glBlendFunc ( GL_ONE, GL_ONE );
		glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glEnable ( GL_BLEND );

		m_cubeShader->bind();
		m_cubeShader->setUniform("mvp", mvp);
		m_cubeShader->drawIndexed ( GL_LINES, 0, data->c_indices.cols() );

		m_camShader->bind();
		m_camShader->setUniform("mvp", mvp);
		m_camShader->drawArray(GL_LINES, 0, data->e_vertices.cols());

		glEnable ( GL_PROGRAM_POINT_SIZE );

		m_pointShader->bind();
		m_pointShader->setUniform("mvp", data_mvp);

		// m_pointShader->setUniform("model", model);
		// m_pointShader->setUniform("view", view);
		// m_pointShader->setUniform("proj", proj);
		// m_pointShader->setUniform("tic", tic);

		// if (index != 0 && master_pointShader) {

		// 	m_pointShader->shareAttrib (*master_pointShader, "position");
		// 	m_pointShader->shareAttrib (*master_pointShader, "color");

		// }

		m_pointShader->drawArray(GL_POINTS, 0, data->p_vertices.cols());

		glDisable ( GL_PROGRAM_POINT_SIZE );
		glDisable ( GL_BLEND );
		glEnable ( GL_DEPTH_TEST );

		// additional stuff, directly in nanovg

		if (vg) {

			nvgSave(vg);

			// border
			nvgBeginPath(vg);
			nvgStrokeWidth(vg, 1);
			nvgRoundedRect(vg, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1, mSize.y() - 1, 0);
			nvgStrokeColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.1f));
			nvgStroke(vg);

			// caption, bottom-left
			if (!caption.empty()) {

				nvgFontFace(vg, "sans");
				nvgFontSize(vg, 9);
				nvgFontBlur(vg, 0.3f);
				nvgFillColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.5f));
				// nvgText(vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "T [%.1f, %.1f, %.1f], R [%.1f, %.1f, %.1f], C [%.1f, %.1f, %.1f], F [%.1f, %.1f, %.1f], U [%.1f, %.1f, %.1f]", total_translation[0], total_translation[1], total_translation[2], total_rotation[0], total_rotation[1], total_rotation[2], camera[0], camera[1], camera[2], forward[0], forward[1], forward[2], up[0], up[1], up[2] ).c_str(), nullptr);
				nvgText(vg, mPos.x() + 37, mPos.y() + size().y() - 3, string_format ( "[%s]", caption.c_str()).c_str(), nullptr);

			}

			// cam info, bottom-left
			if (!ortho)
				nvgText(vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "FOV: %.1f", fovy).c_str(), nullptr);
			else
				nvgText(vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "ORTHO" ).c_str(), nullptr);

			// bottom-right label
			nvgFontFace(vg, "sans");
			nvgFontSize(vg, 9);
			nvgFontBlur(vg, 0.3f);
			nvgFillColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.5f));
			nvgText(vg, mPos.x() + size().x() - 28, mPos.y() + size().y() - 3, string_format ( "%.1f fps", 1.0f / frame_time ).c_str(), nullptr);

			// top-right label
			// tools
			// nvgFontFace(vg, "sans");
			// nvgFontSize(vg, 9);
			// nvgFontBlur(vg, 0.3f);
			// nvgFillColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.5f));

			// top-left label
			nvgFontFace(vg, "sans");
			nvgFontSize(vg, 9);
			nvgFontBlur(vg, 0.3f);
			nvgFillColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.5f));
			nvgText(vg, mPos.x() + 2, mPos.y() + 7, string_format ( "%d", num_frames ).c_str(), nullptr);

			// draw cameras' coords
			for (int i = 0; i < data->e_vertices.cols(); i += 2) {

				// skip self
				if (i / 2 == index) continue;

				nvgFontSize(vg, 6);

				Eigen::Vector3f coords3f = data->e_vertices.col(i + 1);
				Eigen::Vector3f screen_coords = nanogui::project(coords3f, model * view, proj, size());
				screen_coords[1] = size().y() - screen_coords[1];

				// background
				nvgBeginPath(vg);
				nvgRoundedRect(vg, mPos.x() + screen_coords[0] + 1, mPos.y() + screen_coords[1], 16, 16, 4);
				nvgFillColor(vg, nanogui::Color(0.5f, 0.5f, 0.5f, 0.5f));
				nvgFill(vg);

				Eigen::Vector3f color = data->e_colors.col(i);
				nvgFillColor(vg, nanogui::Color(color[0], color[1], color[2], 0.5f));
				nvgText(vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 5, string_format ( "%1.1f", coords3f[0]).c_str(), nullptr);
				nvgText(vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 10, string_format ( "%1.1f", coords3f[1]).c_str(), nullptr);
				nvgText(vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 15, string_format ( "%1.1f", coords3f[2]).c_str(), nullptr);

			}

			nvgRestore(vg);

		} else {

			/* printf("cam %d: nanovg context == nullptr \n", index); */

		}

		// save sreen
		if (record_interval > 0) {

			// if (num_frames % record_interval == 0) {
			if (record) {
				std::string fname = string_format ( "snapshots/screens/%s_%08d.png", record_prefix.c_str(), num_frames);
				saveScreenShot(false, fname.c_str());
			}
		}

	}

	void saveScreenShot(int premult, const char* name) {

		Eigen::Vector2i fbSize;
		Eigen::Vector2i wsize;

		if (glfw_window) {

			glfwGetFramebufferSize ( glfw_window, &fbSize[0], &fbSize[1] );
			glfwGetWindowSize ( glfw_window, &wsize[0], &wsize[1] );

			float pixel_ratio = (float)fbSize[0] / wsize[0];

			int x = mPos.x() * pixel_ratio;
			int y = fbSize[1] - size().y() * pixel_ratio - 5;
			int w = size().x() * pixel_ratio;
			int h = size().y() * pixel_ratio - 5;

			screenshot(x, y, w, h, premult, name);
		}
	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {  tools->setPosition({size[0] - 110, 0}); return true; }

	~Plot() { /* free resources */

		delete m_pointShader;
		delete m_cubeShader;
		delete m_camShader;

	}

	//data
	PlotData *data;
	nanogui::Widget *tools;

	std::vector<std::pair<int, std::string>> textures;

	size_t local_data_checksum = 0;
	size_t record_interval = 0;
	std::string record_prefix = "";

	bool keyboard_enabled = false;

	// for intercepting keyboard events
	GLFWwindow *glfw_window = nullptr;
	NVGcontext *vg = nullptr;

	// shaders
	nanogui::GLShader *m_pointShader = nullptr;
	nanogui::GLShader *m_cubeShader = nullptr;
	nanogui::GLShader *m_camShader = nullptr;

	nanogui::GLShader *master_pointShader = nullptr;
	nanogui::GLShader *master_cubeShader = nullptr;
	nanogui::GLShader *master_camShader = nullptr;


	//model, view, projection...
	Eigen::Vector3f translation, rotation, camera, forward, right, up, data_translation, box_size, total_translation, total_rotation;
	Eigen::Matrix4f view, proj, model, data_model, mvp, data_mvp;
	Eigen::Matrix4f T, R;
	Eigen::Quaternionf q;

	bool ortho = false;

	float near, far, fovy;
	float cam_speed, cam_angular_speed;

	std::string caption = "";
	int index = 0;

	float frame_time, tic;
	size_t num_frames = 0;
};

#endif
