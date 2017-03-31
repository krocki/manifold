/*
* @Author: Kamil Rocki
* @Date:   2017-03-30 11:02:57
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-30 14:12:08
*/

#ifndef _RNGS_H_
#define _RNGS_H_

#include <rand/pcg32.h>
#include <rand/mtrnd.h>
#include <random>
#include <climits>

class RNG {

		// 		virtual void init() {};
		// 		virtual void get ( Eigen::MatrixXf &rs, size_t num_rands ) = 0;
		
		// };
		
		// template <typename T>
		// class RNG_PCG32 : public RNG<T> {
		
		// 		virtual void get ( Eigen::MatrixXf &rs, size_t num_rands = 1 ) {
		
		// 			rs.resize ( num_rands );
		
		// 			for (size_t i = 0; i < num_rands; i++) {
		
		// 				rs(i) = rng.nextFloat();
		
		// 			}
		// 		}
		
		// 		pcg32 pcg32_rng;
		
};

class RNG_STD : public RNG {

	public:
	
		RNG_STD() : RNG() { init(); }
		
		void init() {
		
			// unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
			
			std::seed_seq seed2{r(), r(), r(), r(), r(), r(), r(), r() };
			std::seed_seq seed3{q(), q(), q(), q(), q(), q(), q(), q() };
			
			e1 = std::default_random_engine ( r() );
			rng_mt19937 = std::mt19937 ( seed2 );
			rng_mt19937_64 = std::mt19937_64 ( seed3 );
			k = std::knuth_b ( r() );
			
			u =  std::uniform_real_distribution<> ( 0, 1 );
			i = std::uniform_int_distribution<> ( 0, RAND_MAX );
			n = std::normal_distribution<> ( 0, 1 );
		};
		
		template <typename T = double, typename F>
		T get ( F distribution ) {
		
			T val = distribution ( e1 );
			return val;
			
		}
		
		template <typename T = double>
		T randn () {
		
			T val = n ( e1 );
			return val;
			
		}
		
		template <typename T = double>
		T rand () {
		
			T val = u ( e1 );
			return val;
			
		}
		
		void get ( Eigen::MatrixXf &rs, size_t num_rands = 1 ) {
		
			//rs.resize ( num_rands );
			
		}
		
		// void canonical ( Eigen::MatrixXf &rs, size_t num_rands = 1 ) {
		
		// 	rs.resize ( num_rands );
		
		// 	for ( int n = 0; n < num_rands; ++n ) {
		
		// 		rs ( n ) = std::generate_canonical<double, num_rands> ( rng_mt19937 );
		// 		std::cout << rs ( n ) << ' ';
		// 	}
		
		// }
		
		void reset() {
		
			//u.reset()
			
			// resets the internal state of the distribution
			// (public member function)
			
		}
		
		//INT_MIN, INT_MAX
		std::random_device r, q;
		
		// rngs
		std::default_random_engine e1;
		std::mt19937 rng_mt19937;
		std::mt19937_64 rng_mt19937_64;
		std::knuth_b k;
		
		// distributions
		
		// uniform
		std::uniform_real_distribution<> u;
		std::uniform_int_distribution<> i;
		
		std::normal_distribution<> n;
		
		/*
				// Bernoulli distributions: produces bool values on a Bernoulli distribution.
				// give "true" p of the time
				// give "false" 1-p of the time
				std::bernoulli_distribution bernoulli ( double p = 0.5 );
		
				// produces integer values on a binomial distribution.
				// The value obtained is the number of successes in a sequence of t yes/no experiments, each of which succeeds with probability p.
				// perform n trials, each succeed with prob p
				std::binomial_distribution binomial ( n = 2, p = 0.5 );
		
				// The value represents the number of failures in a series of independent yes/no trials (each succeeds with probability p), before exactly k successes occur.
				// Pat goes door-to-door selling cookies
				// At each house, there's a p chance that she sells one box
				// how many times will she be turned away before selling n boxes?
				std::negative_binomial_distribution negative_binomial ( n = 2, p = 0.5 );
		
				// The value represents the number of yes/no trials (each succeeding with probability p) which are necessary to obtain a single success.
				// std::geometric_distribution<>(p) is exactly equivalent to std::negative_binomial_distribution<>(1, p). It is also the discrete counterpart of std::exponential_distribution.
				// same as std::negative_binomial_distribution<> d(1, p);
				std::geometric_distribution geometric ( p = 0.5 );
		
				//Poisson distributions
				std::poisson_distribution
				std::exponential_distribution
				std::gamma_distribution
				std::weibull_distribution
				std::extreme_value_distribution
		
				//Normal distributions
		
				std::lognormal_distribution
				std::chi_squared_distribution
				std::cauchy_distribution
				std::fisher_f_distribution
				std::student_t_distribution
		
				//Sampling distributions
				std::discrete_distribution
				std::piecewise_constant_distribution
				std::piecewise_linear_distribution
		*/
		
		
};

RNG_STD rng;


/*
// template <typename T>
// class RNG_MT64 : public RNG<T> {

// void init_rand (void) {

// 	// standard rand()
//     struct timeval t;
//     gettimeofday (&t, NULL);
//     srand ( (unsigned int) ( (t.tv_sec * 1000) + (t.tv_usec / 1000) ) );

// #ifdef USE_MT_RNG
//     // MT RNG
//     unsigned long long init[4]={(unsigned long long) ( (t.tv_sec * 1000) + (t.tv_usec / 1000) ),
//     								23456ULL, 0x34567ULL, 0x45678ULL}, length=4;
//     init_by_array64(init, length);
// #endif

// }

// double get_rand_range(float minimum, float maximum) {

// 	return ( fabs(maximum - minimum) * rand_real01() + minimum);

// }

// double rand_real01(void) {

// #ifdef USE_MT_RNG
// 	return genrand64_real2();
// #else
// 	return get_rand_range(0.0f, 1.0f);
// #endif

// }

// unsigned long long rand_int(void) {

// #ifdef USE_MT_RNG
// 	return genrand64_int64();
// #else
// 	return (unsigned long long)rand();
// #endif

// }

// unsigned long long rand_int_radius (unsigned long long center, unsigned long long radius, unsigned long long max) {

//     unsigned long long range = radius * 2;
//     unsigned long long rand_num = (unsigned long long)((long double)range * rand_real01());

//     center -= radius;
//     center += rand_num;

//     return ((center + max) % max);

// }

// unsigned long long rand_int_radius_2d (unsigned long long center, unsigned long long radius, unsigned long long width, unsigned long long max) {

//     unsigned long long range = radius * 2;
//     unsigned long long rand_num_x = (unsigned long long)((long double)range * rand_real01());
//     unsigned long long rand_num_y = (unsigned long long)((long double)range * rand_real01());

//     center -= radius * width + radius;
//     center += rand_num_x + rand_num_y * width;

//     return ((center + max) % max);

// }

// unsigned long long rand_int_radius_2d_gaussian (unsigned long long center, unsigned long long radius, unsigned long long width, unsigned long long max) {

//     unsigned long long range = radius * 2;
//     unsigned long long rand_num_x = (unsigned long long)((long double)range * gaussrand() * 0.4);
//     unsigned long long rand_num_y = (unsigned long long)((long double)range * gaussrand() * 0.4);

//     center -= radius/2 * width + radius/2;
//     center += rand_num_x + rand_num_y * width;

//     return ((center + max) % max);

// }

// #define PI 3.141592654

// double gaussrand(void)
// {
//     static double U, V;
//     static int phase = 0;
//     double Z;

//     if(phase == 0) {
//         U = (rand() + 1.) / (RAND_MAX + 2.);
//         V = rand() / (RAND_MAX + 1.);
//         Z = sqrt(-2 * log(U)) * sin(2 * PI * V);
//     } else
//         Z = sqrt(-2 * log(U)) * cos(2 * PI * V);

//     phase = 1 - phase;

//     return Z;
// }


// }

*/

#endif