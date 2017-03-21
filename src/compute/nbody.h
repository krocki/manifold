/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 21:17:46
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-21 09:14:34
*/

#ifndef __NBODY_H__
#define __NBODY_H__

namespace nbody {

void calculate_forces(Eigen::MatrixXf& points, Eigen::MatrixXf& velocities, float softening = 0.5f * 1e-1f, float dt = 0.0001f) {

	//#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < (size_t) points.cols(); i++) {

		Eigen::Vector3f p = points.col(i);
		Eigen::Vector3f f = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
		Eigen::Vector3f d;

		for (size_t j = 0; j < (size_t) points.cols(); j++) {

			d = points.col(j) - p;

			float dist = d.squaredNorm() + softening;
			float invDist = 1.0f / dist;
			float invDist3 = invDist * invDist * invDist;

			f += invDist3 * d;

		}

		// integrate v
		velocities.col(i).noalias() += dt * f;

		// integrate position
		points.col(i).noalias() += velocities.col(i) * dt;

	}

}
}
#endif