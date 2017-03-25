/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-24 19:45:40
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-24 19:54:10
*/

#ifndef __OPTIMIZATION_H__
#define __OPTIMIZATION_H__

#include <parameters.h>
#include <assert.h>

template<typename T>
void sgd(Parameters<T> &w, Parameters<T> &grads, const float alpha) {

	assert ( w.matrices.size() == grads.matrices.size());

	//TODO sign?
	for ( size_t i = 0; i < w.matrices.size(); i++ )
		w[i] += alpha * grads[i];

}

#endif  /* __OPTIMIZATION_H__ */