/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:22:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-09 20:43:32
*/

#ifndef __UTIL_MAIN_H__
#define __UTIL_MAIN_H__

// to surpress warnings
#define UNUSED(...) [__VA_ARGS__](){};

#include <sstream>
#include <iomanip>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

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

#endif