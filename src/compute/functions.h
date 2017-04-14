#ifndef ___FUNCTIONS_H___
#define ___FUNCTIONS_H___

#include <utils.h>
#include <rand/rngs.h>

typedef enum gen_type { INDEPENDENT = 0, GRID = 1, STRATIFIED = 2} gen_type;

namespace func1 {

float normal ( float scale, size_t i = 0 ) {

	UNUSED ( i );
	return scale * rng.randn();

}

float uniform ( float scale, size_t i = 0 ) {

	UNUSED ( i );
	return rng.get ( std::uniform_real_distribution<> ( -scale, scale ) );

}

}

namespace func3 {

float hat ( float x, float y ) {

	float t = hypotf ( x, y ) * 1.0;
	float z = ( 1 - t * t ) * expf ( t * t / -2.0 );
	return z;

}

float normal ( float x, float y ) {

	UNUSED ( x, y );
	return func1::normal ( 1 );

}

float uniform ( float x, float y ) {

	UNUSED ( x, y );
	return func1::uniform ( 1 );

}

void set ( const Eigen::Vector3f c, Eigen::MatrixXf &points, size_t N = 1 ) {

	points.resize ( 3, N );
	points.colwise() = c;

}

}

template <class FX, class FY, class FZ, class NX, class NY, class NZ>
void generate_stratified ( FX fx, FY fy, FZ fz, NX nx, NY ny, NZ nz, Eigen::MatrixXf &points, size_t N = 1) {

	points.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		float x = round(rng.get ( fx ));
		float y = round(rng.get ( fy ));
		float z = round(rng.get ( fz ));

		float n_x = rng.get ( nx );
		float n_y = rng.get ( ny );
		float n_z = rng.get ( nz );

		points.col ( i ) << x + n_x, y + n_y, z + n_z;

	}
}

template <class FX, class FY, class FZ>
void generate ( FX fx, FY fy, FZ fz, Eigen::MatrixXf &points, size_t N = 1, gen_type gtype = INDEPENDENT) {

	points.resize ( 3, N );

	int cbrtVal;
	float scale;
	int grid_size = 20;
	int count = 0;

	switch ( gtype ) {

	case INDEPENDENT:
		for ( size_t i = 0; i < N; i++ ) {

			float x = rng.get ( fx );
			float y = rng.get ( fy );
			float z = rng.get ( fz );

			points.col ( i ) << x, y, z;

		}
		break;

	case GRID:

		points.resize ( 3, grid_size * grid_size * grid_size );

		for ( int i = 0; i < grid_size; i++ ) {
			for ( int j = 0; j < grid_size; j++ ) {
				for ( int k = 0; k < grid_size; k++ )

					points.col ( k + j * grid_size + i * grid_size * grid_size ) << ( i + 0.5f ), ( j + 0.5f ), ( k + 0.5f );


			}
		}

		break;

	case STRATIFIED:

		points.resize ( 3, ( grid_size * grid_size * grid_size ) * ( N / ( grid_size * grid_size * grid_size ) ) );

		for ( int i = 0; i < grid_size; i++ ) {
			for ( int j = 0; j < grid_size; j++ ) {
				for ( int k = 0; k < grid_size; k++ ) {
					for ( int l = 0; l < ( int ) N / ( grid_size * grid_size * grid_size ); l++ ) {

						float x = rng.get ( fx );
						float y = rng.get ( fy );
						float z = rng.get ( fz );

						points.col ( count++ ) << ( i + 0.5f + x ), ( j + 0.5f + y ), ( k + 0.5f + z );

					}


				}

			}
		}

		break;

	}

}

template <class Function, class Sampler>
void generate ( Function f, Sampler s, Eigen::MatrixXf &points, size_t N = 1, float scale = 1.0f ) {

	points.resize ( 3, N );

	for ( size_t i = 0; i < N; i++ ) {

		float x = s ( scale, i );
		float y = s ( scale, i );

		points.col ( i ) << x, y, scale *f ( x, y );

	}
}

#endif