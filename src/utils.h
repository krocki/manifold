/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:22:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-21 15:16:00
*/

#ifndef __UTIL_MAIN_H__
#define __UTIL_MAIN_H__

// to surpress warnings
#define UNUSED(...) [__VA_ARGS__](){};

#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

#include <Eigen/Dense>

template <typename T>
std::string to_string_with_precision(const T a_value, const int m = 12, const int n = 5) {
	std::ostringstream out;
	out << std::fixed << std::setw( m ) << std::setprecision( n ) << std::setfill( '_' ) << a_value;
	return out.str();
}

std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::string str;
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

// rands
float rand_float ( float mn, float mx ) {

	float r = random() / ( float ) RAND_MAX;
	return mn + ( mx - mn ) * r;
}

Eigen::Matrix4f quat_to_mat(const Eigen::Quaternionf &q) {

	Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();
	mat.block(0, 0, 3, 3) = q.matrix();
	return mat;

}

Eigen::Matrix4f translate(const Eigen::Vector3f &v) {
	return Eigen::Affine3f(Eigen::Translation<float, 3>(v)).matrix();
}

Eigen::Quaternionf rotate(const Eigen::Vector3f &angle, const Eigen::Vector3f &forward, const Eigen::Vector3f &up, const Eigen::Vector3f &right) {

	Eigen::AngleAxisf roll(angle[0], forward);
	Eigen::AngleAxisf yaw(angle[1], up);
	Eigen::AngleAxisf pitch(angle[2], right);

	return yaw * pitch * roll;
}

#endif