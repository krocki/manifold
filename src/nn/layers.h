/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:06:37
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-07 21:46:36
*/

#ifndef __LAYERS_H__
#define __LAYERS_H__

#include <nn/nn_utils.h>
#include <nn/opt.h>

//abstract
class Layer {

  public:

	const std::string name;

	//used in forward pass

	Matrix x; //inputs
	Matrix y; //outputs

	//grads, used in backward pass
	Matrix dx;
	Matrix dy;

	Layer ( size_t inputs, size_t outputs, size_t batch_size, std::string _label = "layer" ) : name ( _label ) {

		x = Matrix ( inputs, batch_size );
		y = Matrix ( outputs, batch_size );
		dx = Matrix ( inputs, batch_size );
		dy = Matrix ( outputs, batch_size );

		// avg_batch_activations.resize(y.rows());
		// avg_hidden_activations.resize(y.cols());
		// sparsity_correction.resize(y.rows());

	};

	//need to override these
	virtual void forward() = 0;
	virtual void backward() = 0;
	virtual void resetGrads() {};
	virtual void applyGrads ( float alpha, float decay ) {};

	virtual void reset() {};
	virtual void kick() {};

	virtual void layer_info() { std:: cout << name << std::endl; }
	virtual ~Layer() {};

	virtual void save ( nanogui::Serializer &s ) const {

		s.set ( "x", x );
		s.set ( "y", y );
		s.set ( "dx", dx );
		s.set ( "dy", dy );

	};

	virtual bool load ( nanogui::Serializer &s ) {

		if ( !s.get ( "x", x ) ) return false;
		if ( !s.get ( "y", y ) ) return false;
		if ( !s.get ( "dx", dx ) ) return false;
		if ( !s.get ( "dy", dy ) ) return false;

		return true;
	}

	virtual void sparsify ( float sparsity_penalty = 0.0001f, float sparsity_target = 0.15f ) {

		//if ( adjust_sparsity && sparsity_penalty > 1e-9f ) {
		float eps = 1e-3f;

		Eigen::VectorXf avg_batch_activations = y.rowwise().mean().array() + eps; // hidden num x 1
		Eigen::VectorXf avg_hidden_activations = y.colwise().mean().array() + eps;
		sparsity_target += eps;

		Eigen::VectorXf sparsity_correction =
		    sparsity_penalty *
		    (
		        ( -sparsity_target / avg_batch_activations.array() ) +
		        ( ( 1.0f - sparsity_target ) / ( 1.0f - avg_batch_activations.array() + eps ) ) );

		// std::cout << "avg_batch_activations.array() " << avg_batch_activations.array() << std::endl;
		// std::cout << "-sparsity_target / avg_batch_activations.array() " << -sparsity_target / avg_batch_activations.array() << std::endl;
		// std::cout << "(1.0f - sparsity_target) " << (1.0f - sparsity_target) << std::endl;
		// std::cout << "1.0f - avg_batch_activations.array()" << 1.0f - avg_batch_activations.array() << std::endl;
		// std::cout << "1.0f - avg_batch_activations.array() + eps" << (1.0f - avg_batch_activations.array()) << std::endl;
		// std::cout << "sparsity_correction " << sparsity_correction.mean() << std::endl;

		// filter out NaNs and INFs just in case
		checkNaNInf ( sparsity_correction );
		dy.colwise() += sparsity_correction;

		sparsity_target -= eps;
	}
	//}

	// // sparsity
	// 	bool adjust_sparsity = false;
	// 	float sparsity_penalty = 0.0000f;
	// 	float sparsity_target = 0.2f;



	// count number of operations for perf counters
	long ops;

};

class Linear : public Layer {

  public:

	Matrix W;
	Matrix b;

	Matrix dW;
	Matrix db;

	Matrix mW;
	Matrix mb;

	// Matrix gaussian_noise;
	// bool add_gaussian_noise = false;
	// float bias_leakage = 0.0000001f;

	// void forward() {

	// 	y.setZero();// = b.replicate ( 1, x.cols() );

	// 	if ( add_gaussian_noise ) {

	// 		gaussian_noise.resize ( x.rows(), x.cols() );
	// 		matrix_randn ( gaussian_noise, 0, 0.1 );
	// 		x += gaussian_noise;
	// 	}

	// 	BLAS_mmul ( y, W, x );

	// }

	void forward () {

		y = b.replicate ( 1, x.cols() );
		BLAS_mmul ( y, W, x );

	}

	void backward() {

		dW.setZero();
		BLAS_mmul ( dW, dy, x, false, true );
		db = dy.rowwise().sum();
		dx.setZero();
		BLAS_mmul ( dx, W, dy, true, false );


	}

	Linear ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "fc" ) {

		reset();

		W = Matrix ( outputs, inputs );
		b = Vector::Zero ( outputs );
		double range = sqrt ( 6.0 / double ( inputs + outputs ) );

		mW = Matrix::Zero ( W.rows(), W.cols() );
		mb = Vector::Zero ( b.rows() );
		dW = Matrix::Zero ( W.rows(), W.cols() );
		db = Vector::Zero ( b.rows() );

		//matrix_rand ( W, -range, range );
		// matrix_randn ( W, 0, 0.1f );
		matrix_randn ( W, 0, ( 1.0f ) / sqrtf ( W.rows() + W.cols() ) );

	};

	virtual void reset() {



	}

	void resetGrads() {

		dW = Matrix::Zero ( W.rows(), W.cols() );
		db = Vector::Zero ( b.rows() );
	}

	virtual void kick() {

		Matrix gaussian_noise = Matrix ( W.rows(), W.cols() );
		matrix_randn ( gaussian_noise, 0, ( 0.5f ) / sqrtf ( W.rows() + W.cols() ) );
		W += gaussian_noise;

	}

	virtual void layer_info() { std:: cout << name << ": " << W.rows() << ", " << W.cols() << std::endl; }

	void applyGrads ( float alpha, float decay = 0 ) {

		//adagrad

		float memory_loss = 1e-2f;

		mW.noalias() = mW * ( 1.0f - memory_loss ) + dW.cwiseProduct ( dW );
		mb.noalias() = mb * ( 1.0f - memory_loss ) + db.cwiseProduct ( db );

		W.noalias() = ( 1.0f - decay ) * W + alpha * dW.cwiseQuotient ( mW.unaryExpr ( std::ptr_fun ( sqrt_eps ) ) );
		b.noalias() = ( 1.0f - decay ) * b + alpha * db.cwiseQuotient ( mb.unaryExpr ( std::ptr_fun ( sqrt_eps ) ) );

		// 'plain' fixed learning rate update
		// b.noalias() += alpha * db;
		// W.noalias() += alpha * dW;

		flops_performed += 10 * ( dW.cols() * dW.rows() + db.cols() * db.rows() );
		bytes_read += 7 * ( dW.cols() * dW.rows() + db.cols() * db.rows() );

	}

	// void applyGrads ( opt_type otype, float alpha, float decay = 0.0f ) {

	// 	if (otype == SGD) sgd ( alpha, decay );
	// 	else if (otype == SGD_MOMENTUM) sgd_momentum ( alpha, decay );
	// 	else if (otype == ADAGRAD) adagrad ( alpha, decay );
	// 	else if (otype == ADADELTA) pseudo_adadelta ( alpha, decay );

	// 	b.array() += bias_leakage;
	// }

	// // sgd
	// void sgd ( float alpha, float decay = 0.0f ) {

	// 	W *= ( 1.0f - decay );
	// 	b += alpha * db;
	// 	W += alpha * dW;
	// 	flops_performed += W.size() * 4 + 2 * b.size();
	// 	bytes_read += W.size() * sizeof ( dtype ) * 3;
	// }

	// // sgd
	// void sgd_momentum ( float alpha, float decay = 0.0f ) {

	// 	float momentum = 0.5f;

	// 	mW.array() = momentum * mW.array() + (1 - momentum) * dW.array();
	// 	mb.array() = momentum * mb.array() + (1 - momentum) * db.array();

	// 	W *= ( 1.0f - decay );

	// 	b += alpha * mb;
	// 	W += alpha * mW;

	// 	flops_performed += W.size() * 4 + 2 * b.size();
	// 	bytes_read += W.size() * sizeof ( dtype ) * 3;
	// }

	// // adagrad
	// void adagrad ( float alpha, float decay ) {

	// 	mW.array() += dW.array() * dW.array();
	// 	mb.array() += db.array() * db.array();

	// 	W *= ( 1.0f - decay );

	// 	b.array() += alpha * db.array() / (( mb.array() + 1e-6 )).sqrt().array();
	// 	W.array() += alpha * dW.array() / (( mW.array() + 1e-6 )).sqrt().array();

	// 	flops_performed += W.size() * 6 + 2 * b.size();
	// 	bytes_read += W.size() * sizeof ( dtype ) * 4;
	// }

	// // pseudo adadelta
	// void pseudo_adadelta ( float alpha, float decay ) {

	// 	float rho = 0.9f;

	// 	mW.array() = rho * mW.array() + (1 - rho) * dW.array() * dW.array();
	// 	mb.array() = rho * mb.array() + (1 - rho) * db.array() * db.array();

	// 	W *= ( 1.0f - decay );

	// 	b.array() += alpha * db.array() / (( mb.array() + 1e-6 )).sqrt().array();
	// 	W.array() += alpha * dW.array() / (( mW.array() + 1e-6 )).sqrt().array();

	// 	flops_performed += W.size() * 6 + 2 * b.size();
	// 	bytes_read += W.size() * sizeof ( dtype ) * 4;
	// }


	virtual void save ( nanogui::Serializer &s ) const {

		Layer::save ( s );
		s.set ( "W", W );
		s.set ( "b", b );
		s.set ( "dW", dW );
		s.set ( "db", db );
		// s.set("add_gaussian_noise", add_gaussian_noise);
		// s.set("gaussian_noise", gaussian_noise);

	};

	virtual bool load ( nanogui::Serializer &s ) {

		Layer::load ( s );
		if ( !s.get ( "W", W ) ) return false;
		if ( !s.get ( "b", b ) ) return false;
		if ( !s.get ( "dW", dW ) ) return false;
		if ( !s.get ( "db", db ) ) return false;
		// if (!s.get("add_gaussian_noise", add_gaussian_noise)) return false;
		// if (!s.get("gaussian_noise", gaussian_noise)) return false;

		return true;
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

	virtual void layer_info() { std:: cout << "sigm " << std::endl; }

	Sigmoid ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "sigmoid" ) {};
	~Sigmoid() {};

};

class Softmax : public Layer {

  public:

	void forward () {

		y = softmax ( x );

	}

	void backward() {

		dx = dy - y;
	}


	Softmax ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "softmax" ) {};
	~Softmax() {};

};

class Identity : public Layer {

  public:

	void forward () {

		y = x;

	}

	void backward() {

		dx = dy;

	}

	Identity ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "noop" ) {};
	~Identity() {};

};

class ReLU : public Layer {

  public:

	void forward () {

		y = rectify ( x );

	}

	void backward() {

		dx.array() = derivative_ReLU ( y ).array() * dy.array();

	}

	ReLU ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "relu" ) {};
	~ReLU() {};

};

// Exponential Linear Unit
// http://arxiv.org/pdf/1511.07289v5.pdf

class ELU : public Layer {

  public:

	void forward () {

		y = activation_ELU ( x );

	}

	void backward() {

		dx.array() = derivative_ELU ( y ).array() * dy.array();

	}

	ELU ( size_t inputs, size_t outputs, size_t batch_size ) :
		Layer ( inputs, outputs, batch_size, "elu" ) {};
	~ELU() {};

};

class Dropout : public Layer {

  public:

	const float keep_ratio;
	Matrix dropout_mask;

	void forward ( bool test = false ) {

		if ( test ) // skip at test time

			y = x;

		else {

			Matrix rands = Matrix::Zero ( y.rows(), y.cols() );
			matrix_rand ( rands, 0.0f, 1.0f );

			//dropout mask - 1s - preserved elements
			dropout_mask = ( rands.array() < keep_ratio ).cast <float> ();

			// y = y .* dropout_mask, discard elements where mask is 0
			y.array() = x.array() * dropout_mask.array();

			// normalize, so that we don't have to do anything at test time
			y /= keep_ratio;

		}
	}

	void backward() {

		dx.array() = dy.array() * dropout_mask.array();
	}

	Dropout ( size_t inputs, size_t outputs, size_t batch_size, float _ratio ) :
		Layer ( inputs, outputs, batch_size, "dropout" ),  keep_ratio ( _ratio ) {};
	~Dropout() {};

};

#endif
