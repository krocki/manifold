/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:22:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-09 21:11:18
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
#include <iostream>
#include <cmath>
#include <cfloat>

#include <Eigen/Dense>


std::string return_current_time_and_date(const char* format = "%x %X") {

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), format);
	return ss.str();
}

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

std::vector<std::string> split(std::string in) {
	std::vector<std::string> strings;
	std::istringstream f(in);
	std::string s;
	while (getline(f, s, ' ')) {
		std::cout << s << std::endl;
		strings.push_back(s);
	}

	return strings;
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

void checkNaNInf(Eigen::MatrixXf &m) {

	m = m.unaryExpr([](float elem) { // changed type of parameter
		return (std::isnan(elem) || std::isinf(elem)) ? 0.0 : elem; // return instead of assignment
	});

}

void checkNaNInf(Eigen::VectorXf &v) {

	v = v.unaryExpr([](float elem) { // changed type of parameter
		return (std::isnan(elem) || std::isinf(elem)) ? 0.0 : elem; // return instead of assignment
	});

}

#endif
