/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:09:39
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-20 20:14:26
*/

#ifndef __GLPLOT_H__
#define __GLPLOT_H__

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/glcanvas.h>
#include <gui/gldata.h>

#define POINT_SHADER_NAME "point_shader"
#define POINT_FRAG_FILE "./src/glsl/surf_point.f.glsl"
#define POINT_VERT_FILE "./src/glsl/surf_point.v.glsl"

class Plot : public nanogui::GLCanvas {

  public:

	Plot ( Widget *parent, bool transparent = false ) : nanogui::GLCanvas ( parent, transparent ) {

		data = nullptr;

		m_pointShader = new nanogui::GLShader();
		m_pointShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		m_cubeShader = new nanogui::GLShader();
		m_cubeShader->initFromFiles ( POINT_SHADER_NAME, POINT_VERT_FILE, POINT_FRAG_FILE );

		setBackgroundColor ( nanogui::Color ( 0, 0, 0, 64 ) );
		setDrawBorder ( true );
		setVisible ( true );

		init_camera();

	}

	Plot ( Widget *parent, const Eigen::Vector2i &w_size, bool transparent = false): Plot(parent, transparent) { setSize(w_size); }

	void init_camera() {

		near = 0.1f;
		far = 100.0f;
		fovy = 67.0f;

		cam_speed = 0.05f;
		cam_angular_speed = 0.0001f * 360.0f / M_PI;

		camera = Eigen::Vector3f(0.0f, 0.0f, 5.0f);
		forward = Eigen::Vector3f (0.0f, 0.0f, -1.0f);
		right = Eigen::Vector3f ( 1.0f, 0.0f, 0.0f);
		up = Eigen::Vector3f (0.0f, 1.0f, 0.0f);

		rotation.setZero();
		translation.setZero();
		q.setIdentity();

	}

	void update_projection() {

		float fH = std::tan(fovy / 360.0f * M_PI) * near;
		float fW = fH * (float) mSize.x() / (float) mSize.y();

		proj = nanogui::frustum(-fW, fW, -fH, fH, near, far);

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

		rotation.setZero();
		translation.setZero();

		T = translate({camera[0], camera[1], camera[2]});
		view = R.inverse() * T.inverse();

	}

	void update_model() {

		model.setIdentity();

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

			m_pointShader->bind();
			m_pointShader->uploadAttrib("position", data->p_vertices);
			m_pointShader->uploadAttrib("color", data->p_colors);

			m_cubeShader->bind();
			m_cubeShader->uploadAttrib("position", data->c_vertices);
			m_cubeShader->uploadAttrib("color", data->c_colors);

			local_data_checksum = data->checksum;

		} else {

			/* printf("data is null... not refreshing\n") */;

		}

	}


	void process_keyboard() {

		if (glfw_window) {

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

		update_model();
		update_view();
		update_projection();

		mvp = proj * view * model;

	}

	void drawGL() override {

		process_keyboard();

		/* Upload points to GPU */

		if (data->checksum != local_data_checksum) {

			refresh_data();

		}

		// update model, view, projection
		update_mvp();

		/* Render the point set */

		glDisable ( GL_DEPTH_TEST );
		glEnable ( GL_BLEND );
		glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glPointSize(15);

		m_cubeShader->bind();
		m_cubeShader->setUniform("mvp", mvp);
		m_cubeShader->drawArray(GL_POINTS, 0, data->c_vertices.size());

		glEnable ( GL_PROGRAM_POINT_SIZE );

		m_pointShader->bind();
		m_pointShader->setUniform("mvp", mvp);
		m_pointShader->drawArray(GL_POINTS, 0, data->p_vertices.size());

		glDisable ( GL_PROGRAM_POINT_SIZE );
		glDisable ( GL_BLEND );
		glEnable ( GL_DEPTH_TEST );

	}

	~Plot() { /* free resources */

		delete m_pointShader;
		delete m_cubeShader;

	}

	//data
	PlotData *data;

	size_t local_data_checksum = 0;



	// for intercepting keyboard events
	GLFWwindow *glfw_window = nullptr;

	// shaders
	nanogui::GLShader *m_pointShader = nullptr;
	nanogui::GLShader *m_cubeShader = nullptr;

	//model, view, projection...
	Eigen::Vector3f translation, rotation, camera, forward, right, up;
	Eigen::Matrix4f view, proj, model, mvp;
	Eigen::Matrix4f T, R;
	Eigen::Quaternionf q;
	float near, far, fovy;
	float cam_speed, cam_angular_speed;

};

#endif