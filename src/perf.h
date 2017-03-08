/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 16:20:38
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-07 20:20:57
*/

#ifndef __PERF_H__
#define __PERF_H__

#include <sys/time.h>

// CPU utilization
#define CPU_UTIL_HISTORY_SIZE 50
#define FLOPS_HISTORY_SIZE 50
#define BYTES_HISTORY_SIZE 50

double get_time();

// CPU util
std::vector<float> cpu_util ( CPU_UTIL_HISTORY_SIZE, 0 );
std::vector<float> cpu_flops ( FLOPS_HISTORY_SIZE, 0 );
std::vector<float> cpu_reads ( BYTES_HISTORY_SIZE, 0 );

double last_cpu_time = get_time();
double last_flops_time = get_time();

long double flops_performed = 0;
long double bytes_read = 0;

double start_time;

void init_start_time() {

	start_time = get_time();

}

double time_since_start() {

	return get_time() - start_time;
}

double get_time ( void ) {

	struct timeval tv;
	gettimeofday ( &tv, NULL );
	double t;
	t = tv.tv_sec + tv.tv_usec * 1e-6;
	return t;

}

void tic ( void ) {

	last_cpu_time = get_time();

}

void toc( void ) {

	double t = get_time() - last_cpu_time;

	cpu_util.push_back ( t );
	cpu_util.erase ( cpu_util.begin() );

}

void ticf ( void ) {

	last_flops_time = get_time();
	flops_performed = 0;
	bytes_read = 0;

}

void tocf ( void ) {

	double t = get_time() - last_flops_time;
	double scale_flops = (double)(1 << 30); // 1 GF = 1073741824 F
	double scale_bytes = (double)(1 << 20); // 1 MB

	cpu_flops.push_back ((flops_performed / scale_flops) / t);
	cpu_flops.erase ( cpu_flops.begin() );

	cpu_reads.push_back ((bytes_read / scale_bytes) / t);
	cpu_reads.erase ( cpu_reads.begin() );
}

#endif