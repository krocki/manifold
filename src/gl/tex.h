/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-06 13:20:16
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-26 11:10:28
*/
#ifndef _GL_TEX_
#define _GL_TEX_

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stb_image.h>


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

void screenshot(int x, int y, int w, int h, int premult, const char* name) {

	unsigned char* image = (unsigned char*)malloc(w * h * 4);
	if (image == NULL)
		return;
	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
	if (premult)
		unpremultiplyAlpha(image, w, h, w * 4);
	else
		setAlpha(image, w, h, w * 4, 255);
	flipHorizontal(image, w, h, w * 4);
	stbi_write_png(name, w, h, 4, image, w * 4);
	free(image);
}

int nvgCreateImageA(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data) {

	return nvgInternalParams(ctx)->renderCreateTexture(nvgInternalParams(ctx)->userPtr, NVG_TEXTURE_ALPHA, w, h, imageFlags, data);

}

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

#endif
