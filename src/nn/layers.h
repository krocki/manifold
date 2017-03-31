/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-03 15:06:37
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-31 10:40:59
*/

#ifndef __LAYERS_H__
#define __LAYERS_H__

#include <nn/nn_utils.h>
#include <nn/opt.h>

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
	virtual void applyGrads ( opt_type otype, float alpha, float decay = 0.0f ) { UNUSED ( alpha, decay, otype ); };

	virtual void layer_info() { std:: cout << "layer " << std::endl; }
	virtual ~Layer() {};

	virtual void save(nanogui::Serializer &s) const {

		s.set("x", x);
		s.set("y", y);
		s.set("dx", dx);
		s.set("dy", dy);

	};

	virtual bool load(nanogui::Serializer &s) {

		if (!s.get("x", x)) return false;
		if (!s.get("y", y)) return false;
		if (!s.get("dx", dx)) return false;
		if (!s.get("dy", dy)) return false;

		return true;
	}

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

	Matrix gaussian_noise;
	bool add_gaussian_noise = false;

	void forward() {

		y = b.replicate ( 1, x.cols() );

		if ( add_gaussian_noise ) {

			gaussian_noise.resize ( x.rows(), x.cols() );
			matrix_randn ( gaussian_noise, 0, 0.1 );
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
		mW = Matrix::Zero ( outputs, inputs );
		mb = Vector::Zero ( outputs );

		matrix_randn ( W, 0, ( 1.0f ) / sqrtf ( W.rows() + W.cols() ) );

	};

	virtual void layer_info() { std:: cout << "lin " << W.rows() << ", " << W.cols() << std::endl; }

	void resetGrads() {

		dW = Matrix::Zero ( W.rows(), W.cols() );
		db = Vector::Zero ( b.rows() );
	}

	// pseudo adadelta
	// void applyGrads ( float alpha, float decay = 0.0f ) {

	// 	float rho = 0.99f;

	// 	mW.array() = rho * mW.array() + (1 - rho) * dW.array() * dW.array();
	// 	mb.array() = rho * mb.array() + (1 - rho) * db.array() * db.array();

	// 	W *= ( 1.0f - decay );

	// 	b.array() += alpha * db.array() / (( mb.array() + 1e-6 )).sqrt().array();
	// 	W.array() += alpha * dW.array() / (( mW.array() + 1e-6 )).sqrt().array();

	// 	flops_performed += W.size() * 4 + 2 * b.size();
	// 	bytes_read += W.size() * sizeof ( dtype ) * 3;
	// }


	void applyGrads ( opt_type otype, float alpha, float decay = 0.0f ) {

		if (otype == SGD) sgd ( alpha, decay );
		else if (otype == ADAGRAD) adagrad ( alpha, decay );
		else if (otype == ADADELTA) pseudo_adadelta ( alpha, decay );
	}

	// sgd
	void sgd ( float alpha, float decay = 0.0f ) {

		W *= ( 1.0f - decay );
		b += alpha * db;
		W += alpha * dW;
		flops_performed += W.size() * 4 + 2 * b.size();
		bytes_read += W.size() * sizeof ( dtype ) * 3;
	}

	// adagrad
	void adagrad ( float alpha, float decay ) {

		mW.array() += dW.array() * dW.array();
		mb.array() += db.array() * db.array();

		W *= ( 1.0f - decay );

		b.array() += alpha * db.array() / (( mb.array() + 1e-6 )).sqrt().array();
		W.array() += alpha * dW.array() / (( mW.array() + 1e-6 )).sqrt().array();

		flops_performed += W.size() * 6 + 2 * b.size();
		bytes_read += W.size() * sizeof ( dtype ) * 4;
	}

	// pseudo adadelta
	void pseudo_adadelta ( float alpha, float decay ) {

		float rho = 0.95f;

		mW.array() = rho * mW.array() + (1 - rho) * dW.array() * dW.array();
		mb.array() = rho * mb.array() + (1 - rho) * db.array() * db.array();

		W *= ( 1.0f - decay );

		b.array() += alpha * db.array() / (( mb.array() + 1e-6 )).sqrt().array();
		W.array() += alpha * dW.array() / (( mW.array() + 1e-6 )).sqrt().array();

		flops_performed += W.size() * 6 + 2 * b.size();
		bytes_read += W.size() * sizeof ( dtype ) * 4;
	}


	virtual void save(nanogui::Serializer &s) const {

		Layer::save(s);
		s.set("W", W);
		s.set("b", b);
		s.set("dW", dW);
		s.set("db", db);
		s.set("add_gaussian_noise", add_gaussian_noise);
		s.set("gaussian_noise", gaussian_noise);

	};

	virtual bool load(nanogui::Serializer &s) {

		Layer::load(s);
		if (!s.get("W", W)) return false;
		if (!s.get("b", b)) return false;
		if (!s.get("dW", dW)) return false;
		if (!s.get("db", db)) return false;
		if (!s.get("add_gaussian_noise", add_gaussian_noise)) return false;
		if (!s.get("gaussian_noise", gaussian_noise)) return false;

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
	virtual void layer_info() { std:: cout << "relu " << std::endl; }

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

	virtual void layer_info() { std:: cout << "softmax " << std::endl; }

	Softmax ( size_t inputs, size_t outputs, size_t batch_size ) : Layer ( inputs, outputs, batch_size ) {};
	~Softmax() {};

};

class Dropout : public Layer {

  public:

	const float keep_ratio;
	Matrix dropout_mask;

	void forward () {

		// if ( test ) // skip at test time

		// 	y = x;

		// else {

		Matrix rands = Matrix::Zero ( y.rows(), y.cols() );
		matrix_rand ( rands, 0.0f, 1.0f );

		//dropout mask - 1s - preserved elements
		dropout_mask = ( rands.array() < keep_ratio ).cast <float> ();

		// y = y .* dropout_mask, discard elements where mask is 0
		y.array() = x.array() * dropout_mask.array();

		// normalize, so that we don't have to do anything at test time
		y /= keep_ratio;

		// }
	}

	virtual void layer_info() { std:: cout << "dropout " << std::endl; }

	void backward() {

		dx.array() = dy.array() * dropout_mask.array();
	}

	Dropout ( size_t inputs, size_t outputs, size_t batch_size, float _ratio ) :
		Layer ( inputs, outputs, batch_size),  keep_ratio ( _ratio ) {};
	~Dropout() {};

};

#endif
