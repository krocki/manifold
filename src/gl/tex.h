/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-06 13:20:16
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-11 15:52:53
*/
#ifndef _GL_TEX_
#define _GL_TEX_

#include <io/import.h>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stb_image.h>

static int mini ( int a, int b ) { return a < b ? a : b; }

static void unpremultiplyAlpha ( unsigned char *image, int w, int h, int stride ) {
	int x, y;

	// Unpremultiply
	for ( y = 0; y < h; y++ ) {
		unsigned char *row = &image[y * stride];
		for ( x = 0; x < w; x++ ) {
			int r = row[0], g = row[1], b = row[2], a = row[3];
			if ( a != 0 ) {
				row[0] = ( int ) mini ( r * 255 / a, 255 );
				row[1] = ( int ) mini ( g * 255 / a, 255 );
				row[2] = ( int ) mini ( b * 255 / a, 255 );
			}
			row += 4;
		}
	}

	// Defringe
	for ( y = 0; y < h; y++ ) {
		unsigned char *row = &image[y * stride];
		for ( x = 0; x < w; x++ ) {
			int r = 0, g = 0, b = 0, a = row[3], n = 0;
			if ( a == 0 ) {
				if ( x - 1 > 0 && row[-1] != 0 ) {
					r += row[-4];
					g += row[-3];
					b += row[-2];
					n++;
				}
				if ( x + 1 < w && row[7] != 0 ) {
					r += row[4];
					g += row[5];
					b += row[6];
					n++;
				}
				if ( y - 1 > 0 && row[-stride + 3] != 0 ) {
					r += row[-stride];
					g += row[-stride + 1];
					b += row[-stride + 2];
					n++;
				}
				if ( y + 1 < h && row[stride + 3] != 0 ) {
					r += row[stride];
					g += row[stride + 1];
					b += row[stride + 2];
					n++;
				}
				if ( n > 0 ) {
					row[0] = r / n;
					row[1] = g / n;
					row[2] = b / n;
				}
			}
			row += 4;
		}
	}
}

static void setAlpha ( unsigned char *image, int w, int h, int stride, unsigned char a ) {
	int x, y;
	for ( y = 0; y < h; y++ ) {
		unsigned char *row = &image[y * stride];
		for ( x = 0; x < w; x++ )
			row[x * 4 + 3] = a;
	}
}

static void flipHorizontal ( unsigned char *image, int w, int h, int stride ) {
	int i = 0, j = h - 1, k;
	while ( i < j ) {
		unsigned char *ri = &image[i * stride];
		unsigned char *rj = &image[j * stride];
		for ( k = 0; k < w * 4; k++ ) {
			unsigned char t = ri[k];
			ri[k] = rj[k];
			rj[k] = t;
		}
		i++;
		j--;
	}
}

void screenshot ( int x, int y, int w, int h, int premult, const char *name ) {

	unsigned char *image = ( unsigned char * ) malloc ( w * h * 4 );
	if ( image == NULL )
		return;
	glReadPixels ( x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image );
	if ( premult )
		unpremultiplyAlpha ( image, w, h, w * 4 );
	else
		setAlpha ( image, w, h, w * 4, 255 );
	flipHorizontal ( image, w, h, w * 4 );
	stbi_write_png ( name, w, h, 4, image, w * 4 );
	free ( image );
}

int nvgCreateImageA ( NVGcontext *ctx, int w, int h, int imageFlags, const unsigned char *data ) {

	return nvgInternalParams ( ctx )->renderCreateTexture ( nvgInternalParams ( ctx )->userPtr, NVG_TEXTURE_ALPHA, w, h,
	        imageFlags, data );

}

class GLTexture {
  public:
	using handleType = std::unique_ptr<uint8_t[], void ( * ) ( void * )>;
	GLTexture() = default;
	GLTexture ( const std::string &textureName )
		: mTextureName ( textureName ), mTextureId ( 0 ) {}

	GLTexture ( const std::string &textureName, GLint textureId )
		: mTextureName ( textureName ), mTextureId ( textureId ) {}

	GLTexture ( const GLTexture &other ) = delete;
	GLTexture ( GLTexture &&other ) noexcept
		: mTextureName ( std::move ( other.mTextureName ) ),
		  mTextureId ( other.mTextureId ) {
		other.mTextureId = 0;
	}
	GLTexture &operator= ( const GLTexture &other ) = delete;
	GLTexture &operator= ( GLTexture &&other ) noexcept {
		mTextureName = std::move ( other.mTextureName );
		std::swap ( mTextureId, other.mTextureId );
		return *this;
	}
	~GLTexture() noexcept {
		if ( mTextureId )
			glDeleteTextures ( 1, &mTextureId );
	}

	GLuint texture() const { return mTextureId; }
	const std::string &textureName() const { return mTextureName; }

	/**
	*  Load a file in memory and create an OpenGL texture.
	*  Returns a handle type (an std::unique_ptr) to the loaded pixels.
	*/
	handleType load ( const std::string &fileName ) {
		if ( mTextureId ) {
			glDeleteTextures ( 1, &mTextureId );
			mTextureId = 0;
		}
		int force_channels = 0;
		int w, h, n;
		handleType textureData ( stbi_load ( fileName.c_str(), &w, &h, &n, force_channels ), stbi_image_free );
		if ( !textureData )
			throw std::invalid_argument ( "Could not load texture data from file " + fileName );
		glGenTextures ( 1, &mTextureId );
		glBindTexture ( GL_TEXTURE_2D, mTextureId );
		GLint internalFormat;
		GLint format;
		switch ( n ) {
		case 1: internalFormat = GL_R8; format = GL_RED; break;
		case 2: internalFormat = GL_RG8; format = GL_RG; break;
		case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
		case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
		default: internalFormat = 0; format = 0; break;
		}
		glTexImage2D ( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get() );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		return textureData;
	}

  private:
	std::string mTextureName;
	GLuint mTextureId;
};

class Texture {

  public:

	GLuint id;
	std::string name = "";

	bool allocated = false;

	size_t tex_size;
	size_t txs_per_dim;
	size_t total_textures;

	// variable number of textures packed
	Eigen::MatrixXf p_vertices;
	Eigen::VectorXf p_colors;
	Eigen::VectorXf p_labels;
	Eigen::MatrixXf p_texcoords;

	GLint internalFormat;
	GLint format;

	Texture() {

		id = 0;
		// glGenTextures(1, &id);
		// glBindTexture(GL_TEXTURE_2D, id);
	}

	Texture ( std::deque<datapoint> &data, GLint fmt, NVGcontext *nvg, size_t image_size = 0 ) : Texture() {

		load ( data, fmt, nvg, image_size );

	}

	//TODO: clean up
	// put 4 functions into 1

	void update_from_matrix ( Eigen::MatrixXf &m, GLint fmt, NVGcontext *nvg, size_t iw = 0, bool weights = false ) {

		size_t image_size = iw;
		size_t data_size = m.cols();

		if ( weights ) data_size = m.rows();

		if ( fmt == GL_RED ) {

			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> r, g, b, a;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

			a.resize ( image_size, image_size );
			a = a.unaryExpr ( [] ( uint32_t x ) {return 0xFF000000;} );

			size_t sqr_dim = ceil ( sqrtf ( data_size ) );

			rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size * 4 );
			float textures_per_dim = ceil ( sqrtf ( data_size ) );
			Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );

			p_labels.resize ( data_size );
			p_texcoords.resize ( 3, data_size );

			for ( size_t i = 0; i < data_size; i++ ) {

				if ( weights ) {
					float_image =  m.row ( i );
					float_image.array() /= float_image.norm();
					float_image.array() += 0.5f;
				} else
					float_image =  m.col ( i );

				float_image *= 255.0f;

				r = float_image.cast<uint32_t>(); r.resize ( image_size, image_size ); g = r; b = r;
				r = r.unaryExpr ( [] ( uint32_t x ) {return x << 0;} ); g = g.unaryExpr ( [] ( uint32_t x ) {return x << 8;} );
				b = b.unaryExpr ( [] ( uint32_t x ) {return x << 16;} );
				rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size, image_size ) = r + g + b + a;

				p_labels[i] = 0;

				p_texcoords.col ( i ) = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
				                        ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );

			}

			txs_per_dim = ( int ) textures_per_dim;
			total_textures = m.cols();
			tex_size = image_size;
			format = fmt;
			name = "datapoints";

			if ( allocated )
				nvgUpdateImage ( nvg, id, ( unsigned char * ) rgba_image.data() );
			else
				id = ( GLuint ) nvgCreateImageRGBA ( nvg, sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST,
				                                     ( unsigned char * ) rgba_image.data() );



		}

		else {

			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> r, g, b, a;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

			size_t sqr_dim = ceil ( sqrtf ( data_size ) );

			a.resize ( image_size, image_size );
			a = a.unaryExpr ( [] ( uint32_t x ) {return 0xFF000000;} );

			float textures_per_dim = ceil ( sqrtf ( data_size ) );
			Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );
			rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size * 4 );

			p_labels.resize ( data_size );
			p_texcoords.resize ( 3, data_size );

			for ( size_t i = 0; i < data_size; i++ ) {

				// std::cout << i << std::endl;
				if ( weights ) {
					float_image =  m.row ( i );
					float_image.array() /= float_image.norm();
					float_image.array() += 0.5f;
				} else
					float_image =  m.col ( i );
				float_image *= 255.0f;
				float_image.resize ( 3, image_size * image_size );

				// inteleave and combine
				r = float_image.block ( 0, 0, 1, image_size * image_size ).cast<uint32_t>(); r.resize ( image_size, image_size );
				r = r.unaryExpr ( [] ( uint32_t x ) {return x << 0;} );
				g = float_image.block ( 1, 0, 1, image_size * image_size ).cast<uint32_t>(); g.resize ( image_size, image_size );
				g = g.unaryExpr ( [] ( uint32_t x ) {return x << 8;} );
				b = float_image.block ( 2, 0, 1, image_size * image_size ).cast<uint32_t>(); b.resize ( image_size, image_size );
				b = b.unaryExpr ( [] ( uint32_t x ) {return x << 16;} );
				rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size, image_size ) = r + g + b + a;

				p_labels[i] = 0;

				p_texcoords.col ( i ) = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
				                        ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );

			}

			txs_per_dim = ( int ) textures_per_dim;
			total_textures = m.cols();
			tex_size = image_size;
			format = fmt;
			name = "datapoints";

			if ( allocated )
				nvgUpdateImage ( nvg, id, ( unsigned char * ) rgba_image.data() );
			else
				id = ( GLuint ) nvgCreateImageRGBA ( nvg, sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST,
				                                     ( unsigned char * ) rgba_image.data() );



		}

		allocated = true;

	}

	void load ( std::deque<datapoint> &data, GLint fmt, NVGcontext *nvg, size_t iw = 0 ) {

		size_t image_size;

		std::cout << id << std::endl;
		std::cout << iw << std::endl;

		if ( fmt == GL_RED ) {

			if ( iw != 0 )
				image_size = iw;
			else
				image_size = sqrt ( data[0].x.size() );

			std::cout << image_size << std::endl;

			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> r, g, b, a;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

			size_t sqr_dim = ceil ( sqrtf ( data.size() ) );
			a.resize ( image_size, image_size );
			a = a.unaryExpr ( [] ( uint32_t x ) {return 0xFF000000;} );

			rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size * 4 );
			float textures_per_dim = ceil ( sqrtf ( data.size() ) );
			Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );

			p_labels.resize ( data.size() );
			p_texcoords.resize ( 3, data.size() );

			for ( size_t i = 0; i < data.size(); i++ ) {

				// std::cout << i << std::endl;
				float_image =  data[i].x;
				float_image *= 255.0f;

				r = float_image.cast<uint32_t>(); r.resize ( image_size, image_size ); g = r; b = r;
				r = r.unaryExpr ( [] ( uint32_t x ) {return x << 0;} ); g = g.unaryExpr ( [] ( uint32_t x ) {return x << 8;} );
				b = b.unaryExpr ( [] ( uint32_t x ) {return x << 16;} );
				rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size, image_size ) = r + g + b + a;

				p_labels[i] = data[i].y;

				p_texcoords.col ( i ) = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
				                        ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );

			}

			bool realloc = false;
			if (txs_per_dim != ( int ) textures_per_dim) realloc = true;
			if (total_textures != data.size()) realloc = true;
			if (tex_size != image_size) realloc = true;
			if (format != fmt) realloc = true;

			txs_per_dim = ( int ) textures_per_dim;
			total_textures = data.size();
			tex_size = image_size;
			format = fmt;
			name = "datapoints";

			if ( !realloc )
				nvgUpdateImage ( nvg, id, ( unsigned char * ) rgba_image.data() );
			else {
				if (id) {glDeleteTextures ( 1, &id );}
				id = ( GLuint ) nvgCreateImageRGBA ( nvg, sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST,
				                                     ( unsigned char * ) rgba_image.data() );
			}


		} else {

			if ( iw != 0 )
				image_size = iw;
			else
				image_size = sqrt ( data[0].x.size() / 3 );

			Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> rgba;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> r, g, b, a;
			Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> rgba_image;

			size_t sqr_dim = ceil ( sqrtf ( data.size() ) );

			a.resize ( image_size, image_size );
			a = a.unaryExpr ( [] ( uint32_t x ) {return 0xFF000000;} );

			float textures_per_dim = ceil ( sqrtf ( data.size() ) );
			Eigen::MatrixXf float_image = Eigen::MatrixXf ( image_size, image_size );
			rgba_image.resize ( sqr_dim * image_size, sqr_dim * image_size * 4 );

			p_labels.resize ( data.size() );
			p_texcoords.resize ( 3, data.size() );

			for ( size_t i = 0; i < data.size(); i++ ) {

				// std::cout << i << std::endl;
				float_image =  data[i].x;
				float_image *= 255.0f;
				float_image.resize ( 3, image_size * image_size );

				// inteleave and combine
				r = float_image.block ( 0, 0, 1, image_size * image_size ).cast<uint32_t>(); r.resize ( image_size, image_size );
				r = r.unaryExpr ( [] ( uint32_t x ) {return x << 0;} );
				g = float_image.block ( 1, 0, 1, image_size * image_size ).cast<uint32_t>(); g.resize ( image_size, image_size );
				g = g.unaryExpr ( [] ( uint32_t x ) {return x << 8;} );
				b = float_image.block ( 2, 0, 1, image_size * image_size ).cast<uint32_t>(); b.resize ( image_size, image_size );
				b = b.unaryExpr ( [] ( uint32_t x ) {return x << 16;} );
				rgba_image.block ( ( i / sqr_dim ) * image_size, ( i % sqr_dim ) * image_size, image_size, image_size ) = r + g + b + a;

				p_labels[i] = data[i].y;

				p_texcoords.col ( i ) = Eigen::Vector3f ( ( i / ( int ) textures_per_dim ) / textures_per_dim,
				                        ( i % ( int ) textures_per_dim ) / textures_per_dim, 0 );

			}

			bool realloc = false;
			if (txs_per_dim != ( int ) textures_per_dim) realloc = true;
			if (total_textures != data.size()) realloc = true;
			if (tex_size != image_size) realloc = true;
			if (format != fmt) realloc = true;

			txs_per_dim = ( int ) textures_per_dim;
			total_textures = data.size();
			tex_size = image_size;
			format = fmt;
			name = "datapoints";

			if ( !realloc )
				nvgUpdateImage ( nvg, id, ( unsigned char * ) rgba_image.data() );
			else {
				if (id) {glDeleteTextures ( 1, &id );}
				id = ( GLuint ) nvgCreateImageRGBA ( nvg, sqr_dim * image_size, sqr_dim * image_size, NVG_IMAGE_NEAREST,
				                                     ( unsigned char * ) rgba_image.data() );
			}


		}

		allocated = true;
	}

	void load ( const std::string &fname ) {

		/*		int force_channels = 0;
				int w, h, n;
				handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
				if (!textureData)
					throw std::invalid_argument("Could not load texture data from file " + fileName);*/

	}


	Eigen::Vector2f texCoords ( int i ) {

		// return coords of image i

	}

	~Texture() { }

};

// 	// TODO, clean up
// 	//star
// 	int force_channels = 0;
// 	int w, h, n;
// 	std::unique_ptr<uint8_t[], void(*)(void*)> textureData(stbi_load("./images/star.png", &w, &h, &n, force_channels), stbi_image_free);
// 	std::cout << w << ", " << h << ", " << n << ", " << std::endl;

// 	GLuint mTextureId;

// 	glGenTextures(1, &mTextureId);
// 	glBindTexture(GL_TEXTURE_2D, mTextureId);
// 	GLint internalFormat;
// 	GLint format;
// 	switch (n) {
// 	case 1: internalFormat = GL_R8; format = GL_RED; break;
// 	case 2: internalFormat = GL_RG8; format = GL_RG; break;
// 	case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
// 	case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
// 	default: internalFormat = 0; format = 0; break;
// 	}
// 	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

// 	textures[1] = ( std::pair<int, std::string> ( mTextureId, "" ) );

// }

#endif
