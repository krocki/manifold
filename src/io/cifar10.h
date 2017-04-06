/*
* @Author: kmrocki
* @Date:   2016-02-24 10:20:09
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-03 10:51:14
*/

#ifndef __CIFAR_IMPORTER__
#define __CIFAR_IMPORTER__

#include <deque>
#include <fstream>

//set Matrix implementation
#include <Eigen/Dense>
typedef Eigen::VectorXf Vector;

typedef struct {

	Vector x; 	//inputs
	int y; 		//label

} datapoint;

//TODO:

#endif

