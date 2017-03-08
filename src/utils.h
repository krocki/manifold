/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:22:47
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 15:22:51
*/

#ifndef __UTIL_MAIN_H__
#define __UTIL_MAIN_H__

// to surpress warnings
#define UNUSED(...) [__VA_ARGS__](){};

#include <sstream>
#include <iomanip>

template <typename T>
std::string to_string_with_precision(const T a_value, const int m = 12, const int n = 5) {
	std::ostringstream out;
	out << std::fixed << std::setw( m ) << std::setprecision( n ) << std::setfill( '_' ) << a_value;
	return out.str();
}

#endif