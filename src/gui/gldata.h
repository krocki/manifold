/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:11:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-28 12:13:23
*/

#ifndef __PLOTDATA_H__
#define __PLOTDATA_H__

#include <utils.h>
#include <random>

#include <gl/tex.h>
#include <io/import.h>
#include <iostream>

class PlotData {

  public:

	PlotData() {};
	~PlotData() {};

	Eigen::MatrixXf e_vertices, e_colors;
	Eigen::MatrixXf r_vertices;
	Eigen::MatrixXf p_vertices, p_colors;
	Eigen::VectorXf p_labels;
	Eigen::MatrixXf p_texcoords;

	Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> c_indices;
	Eigen::MatrixXf c_vertices, c_colors;

	Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> m_indices;
	Eigen::MatrixXf m_vertices, m_colors, m_texcoords;

	using imagesDataType = std::vector<std::pair<int, std::string>>;
	imagesDataType textures;

	void updated() { checksum++; }

	size_t checksum = 0; // or write update time

	void load_data_textures(std::deque<datapoint>& data, NVGcontext *nvg) {

		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

		size_t image_size = 28;
		size_t sqr_dim = ceil ( sqrtf ( data.size() ) );
		std::cout << sqr_dim * sqr_dim << " " << ceil ( sqr_dim ) << std::endl;

		rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size );
		float textures_per_dim = ceil ( sqrtf ( data.size() ) );
		Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );

		p_labels.resize(data.size());
		p_texcoords.resize(3, data.size());

		for ( size_t i = 0; i < data.size(); i++ ) {

			// std::cout << i << std::endl;
			float_image =  data[i].x;
			float_image.resize ( image_size, image_size );
			float_image *= 255.0f;

			rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size,
			                   image_size ) = float_image.cast<unsigned char>();

			p_labels[i] = data[i].y;

			p_texcoords.col(i) = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
			                                       ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );



		}

		textures.resize ( 2 );

		textures[0] = ( std::pair<int, std::string> ( nvgCreateImageA ( nvg,
		                sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST, ( unsigned char * ) rgba_image.data() ), "" ) );




		// TODO, clean up
		//star
		int force_channels = 0;
		int w, h, n;
		std::unique_ptr<uint8_t[], void(*)(void*)> textureData(stbi_load("./images/star.png", &w, &h, &n, force_channels), stbi_image_free);
		std::cout << w << ", " << h << ", " << n << ", " << std::endl;

		GLuint mTextureId;

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

		textures[1] = ( std::pair<int, std::string> ( mTextureId, "" ) );

	}

};

void generate_cube(Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>& indices, Eigen::MatrixXf& positions, Eigen::MatrixXf& colors, Eigen::Vector3f t, float r = 1.0f, Eigen::Vector3f color = Eigen::Vector3f(0.2f, 0.2f, 0.2f)) {

	indices.resize ( 2, 12 );
	positions.resize ( 3, 8 );
	colors.resize ( 3, 12 );

	indices.col ( 0 ) << 0, 1;
	indices.col ( 1 ) << 1, 2;
	indices.col ( 2 ) << 2, 3;
	indices.col ( 3 ) << 3, 0;
	indices.col ( 4 ) << 4, 5;
	indices.col ( 5 ) << 5, 6;
	indices.col ( 6 ) << 6, 7;
	indices.col ( 7 ) << 7, 4;
	indices.col ( 8 ) << 0, 4;
	indices.col ( 9 ) << 1, 5;
	indices.col ( 10 ) << 2, 6;
	indices.col ( 11 ) << 3, 7;

	positions.col ( 0 ) << -r,  r,  r;
	positions.col ( 1 ) << -r,  r, -r;
	positions.col ( 2 ) <<  r,  r, -r;
	positions.col ( 3 ) <<  r,  r,  r;
	positions.col ( 4 ) << -r, -r,  r;
	positions.col ( 5 ) << -r, -r, -r;
	positions.col ( 6 ) <<  r, -r, -r;
	positions.col ( 7 ) <<  r, -r,  r;

	// translation
	positions.colwise() += t;
	colors.colwise() = color;
};

#include <iostream>

void generate_mesh(Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>& indices, Eigen::MatrixXf& positions, Eigen::MatrixXf& texcoords, Eigen::MatrixXf& colors, Eigen::Vector3f t, float r = 1.0f, Eigen::Vector3f color = Eigen::Vector3f(1.0f, 1.0f, 1.0f)) {

	int i = 1;
	int j = 1;

	indices.resize ( 3, i * j * 2 );
	positions.resize ( 3, (j + 1) * (i + 1) );
	colors.resize ( 3, (j + 1) * (i + 1) );
	texcoords.resize(3, (j + 1) * (i + 1));

	for (int l = 0; l < j + 1; l++) {
		for (int k = 0; k < i + 1; k++) {

			positions.col ( k + (i + 1) * l) << r*k,  r*l,  0;
			texcoords.col ( k + (i + 1) * l) << k,  l,  0;

		}
	}

	for (int l = 0; l < j; l++) {
		for (int k = 0; k < i; k++) {

			indices.col ( k + l * i ) << k + (i + 1) * l , k + (i + 1) * l + 1 , k + (i + 1) * (l + 1);
			indices.col ( i * j + k + l * i ) << k + (i + 1) * l + 1 , k + (i + 1) * (l + 1) + 1 , k + (i + 1) * (l + 1);
		}
	}
	// translation

	positions.colwise() -= Eigen::Vector3f(i * r / 2, j * r / 2, 0);
	positions.colwise() += t;

	for (int l = 0; l < j + 1; l++) {
		for (int k = 0; k < i + 1; k++) {

			colors.col ( k + (i + 1) * l ) = color;

		}
	}
};

#endif
