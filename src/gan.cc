/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-12 23:37:42
*/

#include <thread>
#include <unistd.h>
#include <ctime>

//nanogui
#include <nanogui/screen.h>
#include <serializer.h>

//for NN
#include <io/import.h>
#include <nn/gan.h>

std::shared_ptr<NN> nn;
std::shared_ptr<NN> discriminator;

std::deque<datapoint> train_data;
std::deque<datapoint> reconstruction_data;
std::deque<datapoint> sample_reconstruction_data;
std::deque<datapoint> test_data;

typedef struct datapoints {

	Eigen::MatrixXf x;
	Eigen::MatrixXf y;

} datapoints;

typedef struct gan_train_data_type {

	datapoints noise;

	datapoints gen;
	datapoints real;
	datapoints mixed;

	datapoints generator_dy;

	Eigen::VectorXi training_set_indices;
	Eigen::VectorXi mix_indices;

	bool rgba = false;
	size_t image_size = 28;

} gan_train_data_type;

gan_train_data_type gan_train_data;

//GUI
#include "gui/ganscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	PlotData *gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-3;
	float decay = 0;
	const size_t batch_size = 256;
	const int input_width = static_cast<int> ( train_data[0].x.size() );
	assert ( input_width > 0 );

	size_t e = 0;

	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, {3, 100, input_width } ) );
	discriminator = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, MLP, {input_width, 100, 1} ) );

	nn->otype = SGD;
	nn->pause = true;
	discriminator->otype = SGD;
	discriminator->pause = true;

	size_t generate_point_count = 10000;
	generate ( std::uniform_real_distribution<> ( 0, 1 ),
	           std::uniform_real_distribution<> ( 0, 1 ),
	           std::uniform_real_distribution<> ( 0, 1 ),
	           gl_data->s_vertices, generate_point_count, INDEPENDENT );

	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->s_colors, generate_point_count );

	if ( screen )
		if ( screen->nnview )
			screen->nnview->setnets ( nn, discriminator );

	//bind graph data
	if ( screen ) {
		if ( screen->graph_loss )
			discriminator->loss_data = screen->graph_loss->values_ptr();

	}

	size_t iters = 0;
	float discriminator_loss, smooth_discriminator_loss = -1.0f;

	/* work until main window is open */
	Eigen::MatrixXf generated_mask;

	while ( screen->getVisible() ) {

		// generate ( std::uniform_real_distribution<> ( 0, 1 ), std::uniform_real_distribution<> ( 0, 1 ),
		//            std::uniform_real_distribution<> ( 0, 1 ), gan_train_data.noise.x, batch_size, INDEPENDENT );

		generate ( std::normal_distribution<> ( 0, 15 ), std::normal_distribution<> ( 0, 15 ),
		           std::normal_distribution<> ( 0, 15 ), gan_train_data.noise.x, batch_size, INDEPENDENT );



		nn->forward ( gan_train_data.noise.x );

		//generated data
		gan_train_data.gen.x = nn->layers.back()->y;
		gan_train_data.gen.y.resize ( 1, gan_train_data.gen.x.cols() );
		gan_train_data.gen.y.setZero(); // set labels to 0 - images which came from generator

		//real data
		gan_train_data.real.x.resize ( gan_train_data.gen.x.rows(), gan_train_data.gen.x.cols() );
		gan_train_data.real.y.resize ( 1, gan_train_data.gen.x.cols() );
		gan_train_data.training_set_indices.resize ( batch_size );
		matrix_randi ( gan_train_data.training_set_indices, 0, train_data.size() - 1 );
		make_batch ( gan_train_data.real.x, train_data, gan_train_data.training_set_indices );
		gan_train_data.real.y.setOnes();

		//mix
		gan_train_data.mix_indices.resize ( batch_size );
		matrix_randi ( gan_train_data.mix_indices, 0, 1 );
		gan_train_data.mixed.x = gan_train_data.gen.x;
		gan_train_data.mixed.y = gan_train_data.gen.y;
		mix ( gan_train_data.mixed.x, gan_train_data.real.x, gan_train_data.mix_indices );
		mix ( gan_train_data.mixed.y, gan_train_data.real.y, gan_train_data.mix_indices );

		// discriminator forward pass
		discriminator->forward ( gan_train_data.mixed.x );

		// loss
		discriminator_loss = cross_entropy ( discriminator->layers.back()->y, gan_train_data.mixed.y );
		smooth_discriminator_loss = smooth_discriminator_loss < 0 ? discriminator_loss : 0.99 * smooth_discriminator_loss + 0.01
		                            * discriminator_loss;

		//update graph data
		if ( iters % 100 == 0 && discriminator->loss_data ) {

			discriminator->loss_data->head ( discriminator->loss_data->size() - 1 ) = discriminator->loss_data->tail (
			            discriminator->loss_data->size() - 1 );
			discriminator->loss_data->tail ( 1 ) ( 0 ) = smooth_discriminator_loss;

		}

		// discriminator backward pass
		discriminator->backward ( gan_train_data.mixed.y - discriminator->layers.back()->y );

		// set generator grads
		generated_mask = (1.0f - gan_train_data.mix_indices.transpose().replicate ( 784, 1 ).cast<float>().array()).array();
		gan_train_data.generator_dy.x = generated_mask.array() * discriminator->layers[0]->dx.array();
		nn->layers.back()->y  = generated_mask.array() * nn->layers.back()->y.array();

		// generator backward pass
		nn->backward ( -gan_train_data.generator_dy.x );

		// update discriminator weights
		discriminator->update ( learning_rate, decay );

		// update generator weights
		nn->update ( learning_rate, decay );

		iters++;
		if ( iters % 1000 == 0 ) {

			discriminator->clock = true;
			gl_data->updated();
			nn->collect_statistics();
			discriminator->collect_statistics();
		}

		usleep ( 100 );

	}

	return 0;

}

int main ( int /* argc */, char ** /* argv */ ) {

	try {

		/* init GUI */
		nanogui::init();
		screen = std::shared_ptr<GUI> ( new GUI() );

		// launch a compute thread
		std::thread compute_thread ( compute );

		nanogui::mainloop ( 1 );

		compute_thread.join();
		nanogui::shutdown();

	} catch ( const std::runtime_error &e ) {

		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;

	}

	return 0;
}
