/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:06:37
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-03-18 18:13:26
*/

#ifndef __LAYERS_H__
#define __LAYERS_H__

#include <nn/nn_utils.h>

//abstract
class Layer {

	public:
	
		//used in forward pass
		Matrix x; //inputs
		Matrix y; //outputs
		
		//grads, used in backward pass
		Matrix dx;
		Matrix dy;
		
		Layer ( size_t inputs, size_t outputs, size_t batch_size ) {
		
			x = Matrix ( inputs, batch_size );
			y = Matrix ( outputs, batch_size );
			dx = Matrix ( inputs, batch_size );
			dy = Matrix ( outputs, batch_size );
			
		};
		
		//need to override these
		virtual void forward() = 0;
		virtual void backward() = 0;
		virtual void resetGrads() {};
		virtual void applyGrads ( float alpha, float decay = 0.0f ) { UNUSED ( alpha ); UNUSED ( decay ); };
		
		virtual ~Layer() {};
		
		bool isready() { return _ready; }
		
		// count number of operations for perf counters
		long ops;
		bool _ready = false;
		
};

class Linear : public Layer {

	public:
	
		Matrix W;
		Matrix b;
		
		Matrix dW;
		Matrix db;
		
		Matrix gaussian_noise;
		bool add_gaussian_noise = false;
		
		void forward() {
		
			y = b.replicate ( 1, x.cols() );
			
			if ( add_gaussian_noise ) {
			
				gaussian_noise.resize ( x.rows(), x.cols() );
				randn ( gaussian_noise, 0, 1 );
				x += gaussian_noise;
			}
			
			BLAS_mmul ( y, W, x );
			
		}
		
		void backward() {
		
			dW.setZero();
			BLAS_mmul ( dW, dy, x, false, true );
			db = dy.rowwise().sum();
			dx.setZero();
			BLAS_mmul ( dx, W, dy, true, false );
			
			
		}
		
		Linear ( size_t inputs, size_t outputs, size_t batch_size, bool n = false ) : Layer ( inputs, outputs, batch_size ),
			add_gaussian_noise ( n ) {
			
			W = Matrix ( outputs, inputs );
			b = Vector::Zero ( outputs );
			randn ( W, 0, 0.1 );
			
		};
		
		void resetGrads() {
		
			dW = Matrix::Zero ( W.rows(), W.cols() );
			db = Vector::Zero ( b.rows() );
		}
		
		void applyGrads ( float alpha, float decay = 0.0f ) {
		
			W *= ( 1.0f - decay );
			b += alpha * db;
			W += alpha * dW;
			flops_performed += W.size() * 4 + 2 * b.size();
			bytes_read += W.size() * sizeof ( dtype ) * 3;
		}
		
		~Linear() {};
		
};

class Sigmoid : public Layer {

	public:
	
		void forward() {
		
			y = logistic ( x );
			
		}
		
		void backward() {
		
			dx.array() = dy.array() * y.array() * ( 1.0 - y.array() ).array();
			flops_performed += dx.size() * 3;
			bytes_read += x.size() * sizeof ( dtype ) * 2;
			
		}
		
		Sigmoid ( size_t inputs, size_t outputs, size_t batch_size ) : Layer ( inputs, outputs, batch_size ) {};
		~Sigmoid() {};
		
};

class ReLU : public Layer {

	public:
	
		void forward() {
		
			y = rectify ( x );
			
		}
		
		void backward() {
		
			dx.array() = derivative_ReLU ( y ).array() * dy.array();
			
			flops_performed += dy.size();
			bytes_read += dy.size() * sizeof ( dtype );
			
		}
		
		ReLU ( size_t inputs, size_t outputs, size_t batch_size ) : Layer ( inputs, outputs, batch_size ) {};
		~ReLU() {};
		
};

class Softmax : public Layer {

	public:
	
		void forward() {
		
			y = softmax ( x );
			
		}
		
		void backward() {
		
			dx = dy - y;
			
			flops_performed += dy.size() * 2;
			bytes_read += dy.size() * sizeof ( dtype ) * 2;
		}
		
		
		Softmax ( size_t inputs, size_t outputs, size_t batch_size ) : Layer ( inputs, outputs, batch_size ) {};
		~Softmax() {};
		
};

#endif
