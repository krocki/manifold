/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:09:39
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-07 21:28:29
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

#define POINT_TEX_SHADER_NAME "datapoint_tex_shader"
#define POINT_TEX_FRAG_FILE "./src/glsl/tex_point.f.glsl"
#define POINT_TEX_GEOM_FILE "./src/glsl/tex_point.g.glsl"
#define POINT_TEX_VERT_FILE "./src/glsl/tex_point.v.glsl"

#define CAM_SHADER_NAME "cam_shader"
#define CAM_FRAG_FILE "./src/glsl/cam.f.glsl"
#define CAM_GEOM_FILE "./src/glsl/cam.g.glsl"
#define CAM_VERT_FILE "./src/glsl/cam.v.glsl"

#define RAY_SHADER_NAME "ray_shader"
#define RAY_FRAG_FILE "./src/glsl/ray.f.glsl"
#define RAY_GEOM_FILE "./src/glsl/ray.g.glsl"
#define RAY_VERT_FILE "./src/glsl/ray.v.glsl"

#define BOX_SHADER_NAME "box_shader"
#define BOX_FRAG_FILE "./src/glsl/surf_box.f.glsl"
#define BOX_GEOM_FILE "./src/glsl/surf_box.g.glsl"
#define BOX_VERT_FILE "./src/glsl/surf_box.v.glsl"

#define MESH_SHADER_NAME "mesh_shader"
#define MESH_FRAG_FILE "./src/glsl/mesh.f.glsl"
#define MESH_GEOM_FILE "./src/glsl/mesh.g.glsl"
#define MESH_VERT_FILE "./src/glsl/mesh.v.glsl"

class Plot : public nanogui::GLCanvas {

  public:

	Plot ( Widget *parent, std::string _caption, const Eigen::Vector2i &w_size, int i, PlotData *plot_data,
	       bool transparent = false, bool _keyboard_enabled = false, bool _mouse_enabled = false, bool _show_rays = false,
	       GLFWwindow *w = nullptr, NVGcontext *nvg = nullptr, float _fovy = 67.0f,
	       const Eigen::Vector3f _camera = Eigen::Vector3f ( 0.0f, 0.0f, 5.0f ),
	       const Eigen::Vector3f _rotation = Eigen::Vector3f ( 0.0f, 0.0f, 0.0f ),
	       const Eigen::Vector3f _box_size = Eigen::Vector3f ( 1.0f, 1.0f, 1.0f ) , bool _ortho = false, int record_intvl = 0,
	       const std::string r_prefix = "" ) : nanogui::GLCanvas ( parent, transparent ) {

		GLCanvas::setSize ( w_size );
		glfw_window = w;
		keyboard_enabled = _keyboard_enabled;
		mousemotion_enabled = _mouse_enabled;
		show_rays = _show_rays;

		vg = nvg;

		data = nullptr;

		setBackgroundColor ( nanogui::Color ( 64, 64, 64, 64 ) );
		setDrawBorder ( true );
		setVisible ( true );

		forward = Eigen::Vector3f ( 0.0f, 0.0f, -1.0f );
		raydir = forward;
		right = Eigen::Vector3f ( 1.0f, 0.0f, 0.0f );
		up = Eigen::Vector3f ( 0.0f, 1.0f, 0.0f );

		index = i;
		caption = _caption;
		bind_data ( plot_data );

		translation.setZero();
		total_translation.setZero();
		total_rotation.setZero();

		init_camera ( _fovy, _camera, _rotation, _ortho );
		box_size = _box_size;
		init_shaders();

		record_interval = record_intvl;
		record_prefix = r_prefix;
		tic = glfwGetTime();

		m_arcball.setSize ( w_size );

		model_scale = Eigen::Vector3f ( 1, 1, 1 );

		tools = new nanogui::Widget ( this );
		tools->setLayout ( new nanogui::GroupLayout ( 0, 0, 0, 0 ) );

		nanogui::Widget *arrows = new nanogui::Widget ( tools );
		arrows->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 2, 2 ) );

		nanogui::Button *b;

		Eigen::Vector2i bsize = Eigen::Vector2i ( 20, 20 );
		nanogui::Color bcolor = nanogui::Color ( 192, 128, 0, 25 );
		nanogui::Color ccolor = nanogui::Color ( 64, 128, 32, 25 );

		b = arrows->add<nanogui::Button> ( "", ENTYPO_ICON_TRIANGLE_LEFT );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( bcolor );
		b->setCallback ( [&] { translation[0] -= cam_speed; } ); b->setTooltip ( "left" );

		b = arrows->add<nanogui::Button> ( "", ENTYPO_ICON_TRIANGLE_DOWN );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( bcolor );
		b->setCallback ( [&] { translation[1] -= cam_speed; } ); b->setTooltip ( "down" );

		b = arrows->add<nanogui::Button> ( "", ENTYPO_ICON_TRIANGLE_UP );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( bcolor );
		b->setCallback ( [&] { translation[1] += cam_speed;  } ); b->setTooltip ( "up" );

		b = arrows->add<nanogui::Button> ( "", ENTYPO_ICON_TRIANGLE_RIGHT );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( bcolor );
		b->setCallback ( [&] { translation[0] += cam_speed; } ); b->setTooltip ( "right" );

		nanogui::Widget *views = new nanogui::Widget ( tools );
		views->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 2, 2 ) );
		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_PLUS );
		b->setFlags ( nanogui::Button::RadioButton );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( ccolor );
		b->setPushed ( coord_type == 0 );
		b->setCallback ( [&]() { std::cout << "Coords 0: " << std::endl; coord_type = 0;} );
		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_CD );
		b->setFlags ( nanogui::Button::RadioButton );
		b->setFixedSize ( bsize );
		b->setPushed ( coord_type == 1 );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "Coords 1: " << std::endl; coord_type = 1;} );
		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_GLOBE );
		b->setFlags ( nanogui::Button::RadioButton );
		b->setFixedSize ( bsize );
		b->setPushed ( coord_type == 2 );
		b->setBackgroundColor ( ccolor );
		b->setCallback ( [&]() { std::cout << "Coords 2: " << std::endl; coord_type = 2;} );
		b = views->add<nanogui::Button> ( "", ENTYPO_ICON_SWEDEN );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setBackgroundColor ( ccolor );
		b->setPushed ( show_box );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Grid off/on " << std::endl; show_box = state; } );

		nanogui::Widget *shader_tools = new nanogui::Widget ( tools );
		shader_tools->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 2,
		                          2 ) );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MONITOR );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( use_textures );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Textures: " << state << std::endl; use_textures = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MONITOR );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( show_reconstructions );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Reconstructions: " << state << std::endl; show_reconstructions = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MONITOR );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( apply_label_color );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "apply_label_color: " << state << std::endl; apply_label_color = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MONITOR );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( show_samples );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "show samples: " << state << std::endl; show_samples = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_HAIR_CROSS );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( show_rays );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Show Rays: " << state << std::endl; show_rays = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_KEYBOARD );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( keyboard_enabled );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Keyboard: " << state << std::endl; keyboard_enabled = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MOUSE );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( mousemotion_enabled );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Mouse: " << state << std::endl; mousemotion_enabled = state;} );

		b = shader_tools->add<nanogui::Button> ( "", ENTYPO_ICON_MOUSE );
		b->setFlags ( nanogui::Button::ToggleButton );
		b->setFixedSize ( bsize );
		b->setPushed ( mousemotion_enabled );
		b->setChangeCallback ( [&] ( bool state ) { std::cout << "Mouse: " << state << std::endl; mousemotion_enabled = state;} );


		nanogui::Widget *sliders = new nanogui::Widget ( tools );
		sliders->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle, 0,
		                     0 ) );

		new nanogui::Label ( sliders, "FOV", "sans-bold", 7 );

		nanogui::Slider *slider = new nanogui::Slider ( sliders );
		slider->setValue ( 0.5f );
		slider->setFixedWidth ( 80 );
		slider->setCallback ( [&] ( float value ) {

			fovy = ( value * 170.0f );
			std::cout << "fovy: " << fovy << std::endl;

		} );

		new nanogui::Label ( sliders, "CAM", "sans-bold", 7 );
		slider = new nanogui::Slider ( sliders );
		slider->setValue ( cam_speed * 10.0f );
		slider->setFixedWidth ( 80 );
		slider->setCallback ( [&] ( float value ) {
			cam_speed = value / 10.0f; std::cout << "cam speed: " << cam_speed << std::endl;
			cam_angular_speed = ( cam_speed / 1000.0f ) * 360.0f / M_PI;

		} );

		new nanogui::Label ( sliders, "MAG", "sans-bold", 7 );
		slider = new nanogui::Slider ( sliders );
		slider->setValue ( 0.5f );
		slider->setFixedWidth ( 80 );
		slider->setCallback ( [&] ( float value ) {
			float s = powf ( 10, ( ( value - 0.5f ) * 4.0f ) );
			model_scale = Eigen::Vector3f ( s, s, s ); std::cout << "scale: " << model_scale << std::endl;

		} );

		new nanogui::Label ( sliders, "SZ", "sans-bold", 7 );
		slider = new nanogui::Slider ( sliders );
		slider->setValue ( 0.2f );
		slider->setFixedWidth ( 80 );
		slider->setCallback ( [&] ( float value ) {
			float s = value * 10.0f;
			pt_size = s; std::cout << "pt size: " << pt_size << std::endl;

		} );

		new nanogui::Label ( sliders, "A", "sans-bold", 7 );
		slider = new nanogui::Slider ( sliders );
		slider->setValue ( 0.7f );
		slider->setFixedWidth ( 80 );
		slider->setCallback ( [&] ( float value ) {
			alpha = value; std::cout << "alpha: " << alpha << std::endl;

		} );

		tools->setPosition ( {w_size[0] - 91, 0} );

	}

	void init_shaders() {

		m_pointShader = new nanogui::GLShader();
		m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		m_pointTexShader = new nanogui::GLShader();
		m_pointTexShader->initFromFiles ( POINT_TEX_SHADER_NAME, POINT_TEX_VERT_FILE, POINT_TEX_FRAG_FILE,
		                                  POINT_TEX_GEOM_FILE );

		m_samplePointShader = new nanogui::GLShader();
		m_samplePointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		m_samplePointTexShader = new nanogui::GLShader();
		m_samplePointTexShader->initFromFiles ( POINT_TEX_SHADER_NAME, POINT_TEX_VERT_FILE, POINT_TEX_FRAG_FILE,
		                                        POINT_TEX_GEOM_FILE );

		m_cubeShader = new nanogui::GLShader();
		m_cubeShader->initFromFiles ( BOX_SHADER_NAME, BOX_VERT_FILE, BOX_FRAG_FILE, BOX_GEOM_FILE );

		m_camShader = new nanogui::GLShader();
		m_camShader->initFromFiles ( CAM_SHADER_NAME, CAM_VERT_FILE, CAM_FRAG_FILE, CAM_GEOM_FILE );

		m_rayShader = new nanogui::GLShader();
		m_rayShader->initFromFiles ( RAY_SHADER_NAME, RAY_VERT_FILE, RAY_FRAG_FILE, RAY_GEOM_FILE );

		m_meshShader = new nanogui::GLShader();
		m_meshShader->initFromFiles ( MESH_SHADER_NAME, MESH_VERT_FILE, MESH_FRAG_FILE, MESH_GEOM_FILE );

	}

	void init_camera ( float _fovy, const Eigen::Vector3f _camera,  const Eigen::Vector3f _rotation, bool _ortho = false ) {

		ortho = _ortho;

		camera = _camera;
		rotation = _rotation;
		fovy = _fovy;

		q.setIdentity();
		q = rotate ( rotation, forward, up, right ) * q;

		near = 0.1f;
		far = 200.0f;

		cam_speed = 0.1f;
		cam_angular_speed = ( cam_speed / 1000.0f ) * 360.0f / M_PI;

	}

	void update_projection() {

		float fH;
		float fW;

		if ( ortho ) {

			fH = 11.0f;
			fW = 11.0f;
			near = 0.0f;
			far = fH + fW;
			proj = nanogui::ortho ( -fW, fW, -fH, fH, near, far );

		} else {

			fH = std::tan ( fovy / 360.0f * M_PI ) * near;
			fW = fH * ( float ) mSize.x() / ( float ) mSize.y();
			proj = nanogui::frustum ( -fW, fW, -fH, fH, near, far );

		}

	}

	void update_view() {

		// have to update cam position based on m_arcball state somehow

		q = rotate ( rotation, forward, up, right ) * q;
		R = quat_to_mat ( q );

		forward = ( R * ( -1.0f * Eigen::Vector4f::UnitZ() ) ).block ( 0, 0, 3, 1 );
		right = ( R * ( Eigen::Vector4f::UnitX() ) ).block ( 0, 0, 3, 1 );
		up = ( R * ( Eigen::Vector4f::UnitY() ) ).block ( 0, 0, 3, 1 );

		camera += forward * ( -translation[2] );
		camera += up * translation[1];
		camera += right * translation[0];

		total_rotation += rotation;
		total_translation += translation;

		rotation.setZero();
		translation.setZero();

		T = translate ( {camera[0], camera[1], camera[2]} );
		view = R.inverse() * T.inverse();

	}

	void update_model() {

		model.setIdentity();
		data_model = nanogui::scale ( model_scale ) * model; //nanogui::scale(model_scale);
		box_model = model; //nanogui::scale(model_scale);
		data_model = translate ( { -box_size[0] / 2, -box_size[1] / 2, -box_size[2] / 2} ) * data_model;

	}

	void bind_data ( PlotData *d ) {

		data = d;

	}

	void unbind_data() {

		data = nullptr;

	}

	void refresh_data() {

		/* Upload points to GPU */

		if ( data ) {

			// if (index == 0) {

			m_pointShader->bind();
			m_pointShader->uploadAttrib ( "position", data->p_vertices );
			m_pointShader->uploadAttrib ( "color", data->p_colors );

			m_pointTexShader->bind();
			m_pointTexShader->uploadAttrib ( "position", data->p_vertices );
			m_pointTexShader->uploadAttrib ( "color", data->p_colors );
			m_pointTexShader->uploadAttrib ( "texcoords", data->input_data_textures.p_texcoords );

			m_samplePointShader->bind();
			m_samplePointShader->uploadAttrib ( "position", data->s_vertices );
			m_samplePointShader->uploadAttrib ( "color", data->s_colors );

			m_samplePointTexShader->bind();
			m_samplePointTexShader->uploadAttrib ( "position", data->s_vertices );
			m_samplePointTexShader->uploadAttrib ( "color", data->s_colors );
			m_samplePointTexShader->uploadAttrib ( "texcoords", data->sample_reconstruction_textures.p_texcoords );

			m_cubeShader->bind();
			m_cubeShader->uploadIndices ( data->c_indices );
			m_cubeShader->uploadAttrib ( "position", data->c_vertices );
			m_cubeShader->uploadAttrib ( "color", data->c_colors );

			m_meshShader->bind();
			m_meshShader->uploadIndices ( data->m_indices );
			m_meshShader->uploadAttrib ( "position", data->m_vertices );
			m_meshShader->uploadAttrib ( "color", data->m_colors );
			m_meshShader->uploadAttrib ( "texcoords", data->m_texcoords );

			local_data_checksum = data->checksum;

		} else {

			/* printf("data is null... not refreshing\n") */;

		}

	}

	void refresh_camera_positions() {

		// update cam position
		data->e_vertices.col ( 2 * index ) = camera - forward;
		data->e_vertices.col ( 2 * index + 1 ) = camera;

		//update rays
		data->r_vertices.col ( 2 * index ) = camera;
		data->r_vertices.col ( 2 * index + 1 ) = camera + 50 * raydir;

		m_camShader->bind();
		m_camShader->uploadAttrib ( "position", data->e_vertices );
		m_camShader->uploadAttrib ( "color", data->e_colors );

		m_rayShader->bind();
		m_rayShader->uploadAttrib ( "position", data->r_vertices );
		m_rayShader->uploadAttrib ( "color", data->e_colors );

	}

	bool mouseButtonEvent ( const Eigen::Vector2i &p, int button, bool down, int modifiers ) override {

		if ( !GLCanvas::mouseButtonEvent ( p, button, down, modifiers ) ) {
			if ( button == GLFW_MOUSE_BUTTON_1 )
				//m_arcball.button ( p, down );
				mouse_button_pressed = !mouse_button_pressed;
		}
		return true;
	}

	bool mouseMotionEvent ( const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers ) override {

		if ( !GLCanvas::mouseMotionEvent ( p, rel, button, modifiers ) ) {

			if ( mousemotion_enabled ) {

				/* takes mouse position on screen and return ray in world coords */
				Eigen::Vector3f relative_pos = Eigen::Vector3f ( 2.0f * ( float ) ( p[0] - mPos[0] ) / ( float ) mSize[0] - 1.0f,
				                               ( float ) ( -2.0f * ( p[1] - mPos[1] ) ) / ( float ) mSize[0] + 1.0f, 1.0f );

				mouse_last_x = relative_pos[0];
				mouse_last_y = relative_pos[1];
				mouse_last_abs_x = p[0];
				mouse_last_abs_y = p[1];

				update_mouse_overlay();

				//m_arcball.motion ( p );

				// std::cout << m_arcball.matrix() << std::endl;

			}
		}

		return true;

	}

	void update_mouse_overlay() {

		// ray casting
		Eigen::Vector3f ray_nds ( mouse_last_x, mouse_last_y, 1.0f );
		Eigen::Vector4f ray_clip ( mouse_last_x, mouse_last_y, -1.0f, 1.0f );
		Eigen::Vector4f ray_eye = proj.inverse() * ray_clip; ray_eye[2] = -1.0f; ray_eye[3] = 0.0f;
		Eigen::Vector3f world = ( view.inverse() * ray_eye ).head<3>();

		// std::cout << "world: " << std::endl << world << std::endl;
		Eigen::Vector3f world_normalized = world.normalized();
		// std::cout << "normalized: " << std::endl << world_normalized << std::endl;

		raydir = world_normalized;

	}

	void process_keyboard() {

		if ( glfw_window && keyboard_enabled ) {

			float rate = ( ( glfwGetKey ( glfw_window, GLFW_KEY_LEFT_SHIFT ) ) == GLFW_PRESS ) ? 5.0 : 1.0f;

			if (mouse_button_pressed && mousemotion_enabled) {

				rotation[1] -= mouse_last_x * 0.05f;
				rotation[2] += mouse_last_y * 0.05f;

			}

			if ( glfwGetKey ( glfw_window, 'A' ) == GLFW_PRESS ) translation[0] -= cam_speed * rate;
			if ( glfwGetKey ( glfw_window, 'D' ) == GLFW_PRESS ) translation[0] += cam_speed * rate;
			if ( glfwGetKey ( glfw_window, 'E' ) == GLFW_PRESS ) translation[1] -= cam_speed * rate;
			if ( glfwGetKey ( glfw_window, 'Q' ) == GLFW_PRESS ) translation[1] += cam_speed * rate;
			if ( glfwGetKey ( glfw_window, 'W' ) == GLFW_PRESS ) translation[2] -= cam_speed * rate;
			if ( glfwGetKey ( glfw_window, 'S' ) == GLFW_PRESS ) translation[2] += cam_speed * rate;

			if ( glfwGetKey ( glfw_window, 'Z' ) == GLFW_PRESS ) rotation[0] -= cam_angular_speed * rate;
			if ( glfwGetKey ( glfw_window, 'C' ) == GLFW_PRESS ) rotation[0] += cam_angular_speed * rate;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_RIGHT ) == GLFW_PRESS ) rotation[1] -= cam_angular_speed * rate;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_LEFT ) == GLFW_PRESS ) rotation[1] += cam_angular_speed * rate;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_DOWN ) == GLFW_PRESS ) rotation[2] -= cam_angular_speed * rate;
			if ( glfwGetKey ( glfw_window, GLFW_KEY_UP ) == GLFW_PRESS ) rotation[2] += cam_angular_speed * rate;

		}

	}

	void update_mvp() {

		// update model, view, projection

		update_model();
		update_view();
		update_projection();

		mvp = proj * view * model;
		data_mvp = proj * view * data_model;
		box_mvp = proj * view * box_model;

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
		if ( data->checksum != local_data_checksum ) {
			refresh_data();
			record = true;
		}

		refresh_camera_positions();

		update_mvp();

		/* Render */

		glDisable ( GL_DEPTH_TEST );
		glBlendFunc ( GL_ONE, GL_ONE );
		glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glEnable ( GL_BLEND );

		if ( show_box ) {

			m_cubeShader->bind();
			m_cubeShader->setUniform ( "mvp", box_mvp );
			m_cubeShader->drawIndexed ( GL_LINES, 0, data->c_indices.cols() );

		}

		m_camShader->bind();
		m_camShader->setUniform ( "mvp", mvp );
		m_camShader->drawArray ( GL_LINES, 0, data->e_vertices.cols() );

		if ( show_rays ) {

			m_rayShader->bind();
			m_rayShader->setUniform ( "mvp", mvp );
			m_rayShader->drawArray ( GL_LINES, 0, data->r_vertices.cols() );
		}

		// glDisable ( GL_BLEND );

		// m_meshShader->bind();
		// glActiveTexture ( GL_TEXTURE0 );
		// glBindTexture ( GL_TEXTURE_2D, data->input_data_textures.id );

		// m_meshShader->setUniform("mvp", mvp);
		// m_meshShader->drawIndexed ( GL_TRIANGLES, 0, data->m_indices.cols() );

		// glEnable ( GL_BLEND );

		if (show_samples && data->s_vertices.cols() > 0) {

			glEnable ( GL_PROGRAM_POINT_SIZE );
			m_samplePointShader->bind();
			m_samplePointShader->setUniform ( "mvp", data_mvp );
			m_samplePointShader->setUniform ( "coord_type", coord_type );
			m_samplePointShader->setUniform ( "pt_size", pt_size );
			m_samplePointShader->setUniform ( "alpha", alpha );

			m_samplePointShader->drawArray ( GL_POINTS, 0, data->s_vertices.cols() );

			glDisable ( GL_PROGRAM_POINT_SIZE );

			m_samplePointTexShader->bind();

			glActiveTexture ( GL_TEXTURE0 );
			glBindTexture ( GL_TEXTURE_2D, ( data->sample_reconstruction_textures.id ) );

			float textures_per_dim = data->sample_reconstruction_textures.txs_per_dim;

			//star
			// glBindTexture ( GL_TEXTURE_2D, ( std::vector<std::pair<int, std::string>> ( data->textures ) [1] ).first );
			// float textures_per_dim = 1;

			m_samplePointTexShader->setUniform ( "image", 0 );
			m_samplePointTexShader->setUniform ( "view", view );
			m_samplePointTexShader->setUniform ( "proj", proj );
			m_samplePointTexShader->setUniform ( "model", data_model );
			m_samplePointTexShader->setUniform ( "coord_type", coord_type );
			m_samplePointTexShader->setUniform ( "alpha", alpha );
			m_samplePointTexShader->setUniform ( "pt_size", pt_size );
			m_samplePointTexShader->setUniform ( "apply_label_color", (int) apply_label_color);

			float quad_size = 0.005f;
			float radius = sqrtf ( 2 * quad_size );
			float tex_w = 1.0f / ( float ) ( data->sample_reconstruction_textures.txs_per_dim );

			m_samplePointTexShader->setUniform ( "radius", radius );
			m_samplePointTexShader->setUniform ( "tex_w", tex_w );

			glPointSize ( 1 );
			glEnable ( GL_DEPTH_TEST );

			glEnable ( GL_BLEND );
			glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			m_samplePointTexShader->drawArray ( GL_POINTS, 0, data->s_vertices.cols() );

			glDisable ( GL_BLEND );
			glDisable ( GL_DEPTH_TEST );

		}

		if ( !use_textures ) {

			glEnable ( GL_PROGRAM_POINT_SIZE );
			m_pointShader->bind();
			m_pointShader->setUniform ( "mvp", data_mvp );
			m_pointShader->setUniform ( "coord_type", coord_type );
			m_pointShader->setUniform ( "pt_size", pt_size );
			m_pointShader->setUniform ( "alpha", alpha );

			m_pointShader->drawArray ( GL_POINTS, 0, data->p_vertices.cols() );

			glDisable ( GL_PROGRAM_POINT_SIZE );
			glDisable ( GL_BLEND );
			glEnable ( GL_DEPTH_TEST );

		} else {

			m_pointTexShader->bind();

			glActiveTexture ( GL_TEXTURE0 );
			if (show_reconstructions)
				glBindTexture ( GL_TEXTURE_2D, ( data->input_reconstruction_textures.id ) );
			else
				glBindTexture ( GL_TEXTURE_2D, ( data->input_data_textures.id ) );

			float textures_per_dim = data->input_data_textures.txs_per_dim;

			//star
			// glBindTexture ( GL_TEXTURE_2D, ( std::vector<std::pair<int, std::string>> ( data->textures ) [1] ).first );
			// float textures_per_dim = 1;

			m_pointTexShader->setUniform ( "image", 0 );
			m_pointTexShader->setUniform ( "view", view );
			m_pointTexShader->setUniform ( "proj", proj );
			// m_pointTexShader->setUniform ( "selected", selected );
			m_pointTexShader->setUniform ( "model", data_model );
			m_pointTexShader->setUniform ( "coord_type", coord_type );
			m_pointTexShader->setUniform ( "alpha", alpha );
			m_pointTexShader->setUniform ( "pt_size", pt_size );
			m_pointTexShader->setUniform ( "apply_label_color", (int) apply_label_color);

			float quad_size = 0.005f;
			float radius = sqrtf ( 2 * quad_size );
			float tex_w = 1.0f / ( float ) ( data->input_data_textures.txs_per_dim );

			m_pointTexShader->setUniform ( "radius", radius );
			m_pointTexShader->setUniform ( "tex_w", tex_w );

			glPointSize ( 1 );
			glEnable ( GL_DEPTH_TEST );

			glEnable ( GL_BLEND );
			glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			m_pointTexShader->drawArray ( GL_POINTS, 0, data->p_vertices.cols() );

			glDisable ( GL_BLEND );
			glDisable ( GL_DEPTH_TEST );


		}

		// additional stuff, directly in nanovg

		if ( vg ) {

			nvgSave ( vg );

			// border
			nvgBeginPath ( vg );
			nvgStrokeWidth ( vg, 1 );
			nvgRoundedRect ( vg, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1, mSize.y() - 1, 0 );
			nvgStrokeColor ( vg, nanogui::Color ( 1.0f, 1.0f, 1.0f, 0.1f ) );
			nvgStroke ( vg );

			if (mouse_button_pressed && mousemotion_enabled) {

				float len = sqrtf((mouse_last_x * mouse_last_x + mouse_last_y * mouse_last_y) / 2.0);

				nvgBeginPath ( vg );
				nvgStrokeWidth ( vg, 1 );
				nvgMoveTo ( vg, mPos.x() + mSize.x() / 2, mPos.y() + mSize.y() / 2);
				nvgLineTo ( vg, mPos.x() + mouse_last_abs_x - 3, mPos.y() + mouse_last_abs_y - 3);
				nvgStrokeColor ( vg, nanogui::Color ( 1.0f, 1.0f, 1.0f, len ) );
				nvgStroke ( vg );

			}

			// caption, bottom-left
			if ( !caption.empty() ) {

				nvgFontFace ( vg, "sans" );
				nvgFontSize ( vg, 9 );
				nvgFontBlur ( vg, 0.3f );
				nvgFillColor ( vg, nanogui::Color ( 1.0f, 1.0f, 1.0f, 0.5f ) );
				// nvgText(vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "T [%.1f, %.1f, %.1f], R [%.1f, %.1f, %.1f], C [%.1f, %.1f, %.1f], F [%.1f, %.1f, %.1f], U [%.1f, %.1f, %.1f]", total_translation[0], total_translation[1], total_translation[2], total_rotation[0], total_rotation[1], total_rotation[2], camera[0], camera[1], camera[2], forward[0], forward[1], forward[2], up[0], up[1], up[2] ).c_str(), nullptr);
				nvgText ( vg, mPos.x() + 37, mPos.y() + size().y() - 3, string_format ( "[%s]", caption.c_str() ).c_str(), nullptr );

			}

			// cam info, bottom-left
			if ( !ortho )
				nvgText ( vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "FOV: %.1f", fovy ).c_str(), nullptr );
			else
				nvgText ( vg, mPos.x() + 3, mPos.y() + size().y() - 3, string_format ( "ORTHO" ).c_str(), nullptr );

			nvgText ( vg, mPos.x() + 95, mPos.y() + size().y() - 3, string_format ( "mouse at [%.3f, %.3f]", mouse_last_x,
			          mouse_last_y ).c_str(), nullptr );

			// bottom-right label
			nvgFontFace ( vg, "sans" );
			nvgFontSize ( vg, 9 );
			nvgFontBlur ( vg, 0.3f );
			nvgFillColor ( vg, nanogui::Color ( 1.0f, 1.0f, 1.0f, 0.5f ) );
			nvgText ( vg, mPos.x() + size().x() - 28, mPos.y() + size().y() - 3, string_format ( "%.1f fps",
			          1.0f / frame_time ).c_str(), nullptr );

			// top-right label
			// tools
			// nvgFontFace(vg, "sans");
			// nvgFontSize(vg, 9);
			// nvgFontBlur(vg, 0.3f);
			// nvgFillColor(vg, nanogui::Color(1.0f, 1.0f, 1.0f, 0.5f));

			// top-left label
			nvgFontFace ( vg, "sans" );
			nvgFontSize ( vg, 9 );
			nvgFontBlur ( vg, 0.3f );
			nvgFillColor ( vg, nanogui::Color ( 1.0f, 1.0f, 1.0f, 0.5f ) );
			nvgText ( vg, mPos.x() + 2, mPos.y() + 7, string_format ( "%d", num_frames ).c_str(), nullptr );

			// draw cameras' coords
			for ( int i = 0; i < data->e_vertices.cols(); i += 2 ) {

				// skip self
				if ( i / 2 == index ) continue;

				nvgFontSize ( vg, 6 );

				Eigen::Vector3f coords3f = data->e_vertices.col ( i + 1 );
				Eigen::Vector3f screen_coords = nanogui::project ( coords3f, model * view, proj, size() );
				screen_coords[1] = size().y() - screen_coords[1];

				// background
				nvgBeginPath ( vg );
				nvgRoundedRect ( vg, mPos.x() + screen_coords[0] + 1, mPos.y() + screen_coords[1], 16, 16, 4 );
				nvgFillColor ( vg, nanogui::Color ( 0.5f, 0.5f, 0.5f, 0.5f ) );
				nvgFill ( vg );

				Eigen::Vector3f color = data->e_colors.col ( i );
				nvgFillColor ( vg, nanogui::Color ( color[0], color[1], color[2], 0.5f ) );
				nvgText ( vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 5, string_format ( "%1.1f",
				          coords3f[0] ).c_str(), nullptr );
				nvgText ( vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 10, string_format ( "%1.1f",
				          coords3f[1] ).c_str(), nullptr );
				nvgText ( vg, mPos.x() + screen_coords[0] + 5, mPos.y() + screen_coords[1] + 15, string_format ( "%1.1f",
				          coords3f[2] ).c_str(), nullptr );

			}

			nvgRestore ( vg );

		} else {

			/* printf("cam %d: nanovg context == nullptr \n", index); */

		}

		// save sreen
		if ( record_interval > 0 ) {

			// if (num_frames % record_interval == 0) {
			if ( record ) {
				std::string fname = string_format ( "snapshots/screens/%s_%08d.png", record_prefix.c_str(), num_frames );
				saveScreenShot ( false, fname.c_str() );
			}
		}

	}

	void saveScreenShot ( int premult, const char *name ) {

		Eigen::Vector2i fbSize;
		Eigen::Vector2i wsize;

		if ( glfw_window ) {

			glfwGetFramebufferSize ( glfw_window, &fbSize[0], &fbSize[1] );
			glfwGetWindowSize ( glfw_window, &wsize[0], &wsize[1] );

			float pixel_ratio = ( float ) fbSize[0] / wsize[0];

			int x = mPos.x() * pixel_ratio;
			int y = fbSize[1] - size().y() * pixel_ratio - 5;
			int w = size().x() * pixel_ratio;
			int h = size().y() * pixel_ratio - 5;

			screenshot ( x, y, w, h, premult, name );
		}
	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {
		m_arcball.setSize ( size ); tools->setPosition ( {size[0] - 91, 0} );
		return true;
	}

	~Plot() { /* free resources */

		delete m_pointShader;
		delete m_pointTexShader;
		delete m_samplePointShader;
		delete m_samplePointTexShader;
		delete m_cubeShader;
		delete m_camShader;
		delete m_rayShader;
		delete m_meshShader;

	}

	//data
	PlotData *data;
	nanogui::Widget *tools;

	Eigen::Vector3f selected;

	std::vector<std::pair<int, std::string>> textures;

	size_t local_data_checksum = 0;
	size_t record_interval = 0;
	std::string record_prefix = "";

	bool keyboard_enabled = false;
	bool mousemotion_enabled = false;
	bool show_box = true;

	// for intercepting keyboard events
	GLFWwindow *glfw_window = nullptr;
	NVGcontext *vg = nullptr;

	// shaders
	nanogui::GLShader *m_pointShader = nullptr;
	nanogui::GLShader *m_pointTexShader = nullptr;
	nanogui::GLShader *m_samplePointShader = nullptr;
	nanogui::GLShader *m_samplePointTexShader = nullptr;
	nanogui::GLShader *m_cubeShader = nullptr;
	nanogui::GLShader *m_camShader = nullptr;
	nanogui::GLShader *m_rayShader = nullptr;
	nanogui::GLShader *m_meshShader = nullptr;

	nanogui::GLShader *master_pointShader = nullptr;
	nanogui::GLShader *master_pointTexShader = nullptr;
	nanogui::GLShader *master_cubeShader = nullptr;
	nanogui::GLShader *master_camShader = nullptr;
	nanogui::GLShader *master_rayShader = nullptr;

	//model, view, projection...
	Eigen::Vector3f translation, rotation, camera, forward, right, up, data_translation, box_size, total_translation,
	      total_rotation, model_scale;
	Eigen::Matrix4f view, proj, model, data_model, box_model, mvp, data_mvp, box_mvp;
	Eigen::Matrix4f T, R;
	Eigen::Quaternionf q;
	Eigen::Vector3f raydir;

	nanogui::Arcball m_arcball;

	bool ortho = false;
	bool use_textures = false;
	bool show_reconstructions = false;
	bool apply_label_color = false;
	bool show_rays = true;
	bool show_samples = false;
	int coord_type = 0;
	float pt_size = 2.0f;
	float alpha = 0.7f;
	bool mouse_button_pressed = false;

	float near, far, fovy;
	float cam_speed, cam_angular_speed;

	std::string caption = "";
	int index = 0;

	float frame_time, tic;
	size_t num_frames = 0;

	float mouse_last_x = -1, mouse_last_y = - 1;
	float mouse_last_abs_x = 0, mouse_last_abs_y = 0;
};

#endif
