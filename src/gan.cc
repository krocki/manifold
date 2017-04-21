/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-20 22:14:02
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
std::shared_ptr<NN> encoder;

std::deque<datapoint> train_data;
std::deque<datapoint> reconstruction_data;
std::deque<datapoint> sample_reconstruction_data;
std::deque<datapoint> test_data;

typedef struct datapoints {

	Eigen::MatrixXf x;
	Eigen::MatrixXf y;

} datapoints;

typedef struct gan_train_data_type {

	datapoints noise, noise_n;
	datapoints gen, real, mixed, mixed_inv, smoothed;
	datapoints generator_dy;

	Eigen::MatrixXf generated_mask, real_mask;
	Eigen::MatrixXi training_set_indices, mix_indices;

	bool rgba = false;
	size_t image_size = 28;

} gan_train_data_type;

gan_train_data_type gan_train_data;

//GUI
#include "gui/ganscreen.h"
#include <compute/functions.h>

std::shared_ptr<GUI> screen;

void update_graph_data ( Eigen::VectorXf *data, float new_val ) {


	if ( data ) {

		data->head ( data->size() - 1 ) = data->tail ( data->size() - 1 );
		data->tail ( 1 ) ( 0 ) = new_val;

	}


}

// TODO: reference batch norm, virtual batch norm
// http://www.iangoodfellow.com/slides/2016-12-04-NIPS.pdf

int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	PlotData *gl_data = screen->plot_data;

	// NN stuff
	double learning_rate = 1e-3f;
	float decay = 0;
	const size_t batch_size = 64;
	const int input_width = static_cast<int> ( train_data[0].x.size() );
	assert ( input_width > 0 );

	size_t e = 0;

	size_t code_dims = 3;

	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, {static_cast<int> ( code_dims ), 16, 100, input_width }, RELU ) );
	discriminator = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, MLP, {input_width, 100, 16, 1}, RELU, false ) );
	encoder = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, MLP, {input_width, 100, 16 }, RELU, false ) );
	encoder->layers.push_back ( new Linear ( encoder->layers.back()->y.rows(), code_dims, batch_size ) );

	nn->otype = SGD;
	nn->pause = true;
	discriminator->otype = SGD;
	discriminator->pause = true;
	encoder->otype = SGD;
	encoder->pause = true;
	nn->generator_loss_type = NON_SATURATING_LOSS;
	discriminator->label_smoothing = 0.9f;

	size_t vis_interval = 1000;

	size_t generate_point_count = vis_interval * batch_size;

	generate ( std::normal_distribution<> ( 0, 0.1 ),
	           std::normal_distribution<> ( 0, 0.1 ),
	           std::normal_distribution<> ( 0, 0.1 ),
	           gl_data->p_vertices, generate_point_count, INDEPENDENT );

	func3::set ( {0.5f, 0.5f, 0.5f}, gl_data->p_colors, generate_point_count );
	generate_ndims ( code_dims, std::normal_distribution<> ( 0, 10 ), gan_train_data.noise.x, batch_size, INDEPENDENT );

	generate ( std::uniform_real_distribution<> ( -40, 40 ),
	           std::uniform_real_distribution<> ( -40, 40 ),
	           std::uniform_real_distribution<> ( -40, 40 ),
	           gl_data->s_vertices, generate_point_count, INDEPENDENT );

	func3::set ( {1.0f, 1.0f, 1.0f}, gl_data->s_colors, generate_point_count );

	if ( screen )
		if ( screen->nnview )
			screen->nnview->setnets ( nn, discriminator );

	//bind graph data
	if ( screen ) {
		if ( screen->graph_loss )
			discriminator->loss_data = screen->graph_loss->values_ptr();
		if ( screen->graph_real_acc )
			discriminator->real_acc_data = screen->graph_real_acc->values_ptr();
		if ( screen->graph_fake_acc )
			discriminator->fake_acc_data = screen->graph_fake_acc->values_ptr();
		if ( screen->graph_encoder_loss )
			encoder->loss_data = screen->graph_encoder_loss->values_ptr();
		if ( screen->graph_generator_w_norm )
			nn->w_norm_data = screen->graph_generator_w_norm->values_ptr();
		if ( screen->graph_discriminator_w_norm )
			discriminator->w_norm_data = screen->graph_discriminator_w_norm->values_ptr();
		if ( screen->graph_generator_dw_norm )
			nn->dw_norm_data = screen->graph_generator_dw_norm->values_ptr();
		if ( screen->graph_discriminator_dw_norm )
			discriminator->dw_norm_data = screen->graph_discriminator_dw_norm->values_ptr();
		if ( screen->graph_generator_mw_norm )
			nn->mw_norm_data = screen->graph_generator_mw_norm->values_ptr();
		if ( screen->graph_discriminator_mw_norm )
			discriminator->mw_norm_data = screen->graph_discriminator_mw_norm->values_ptr();
		if ( screen->graph_discriminator_dy_norm )
			discriminator->in_grad_norm_data = screen->graph_discriminator_dy_norm->values_ptr();
		if ( screen->graph_generator_dy_norm )
			nn->in_grad_norm_data = screen->graph_generator_dy_norm->values_ptr();
		if ( screen->graph_discriminator_dx_norm )
			discriminator->out_grad_norm_data = screen->graph_discriminator_dx_norm->values_ptr();
		if ( screen->graph_generator_dx_norm )
			nn->out_grad_norm_data = screen->graph_generator_dx_norm->values_ptr();


	}

	size_t iters = 0;
	float generator_loss, smooth_generator_loss = -1.0f;
	float encoder_loss, smooth_encoder_loss = -1.0f;
	float discriminator_real_acc, smooth_discriminator_real_acc = -1.0f;
	float discriminator_fake_acc, smooth_discriminator_fake_acc = -1.0f;

	/* work until main window is open */

	int batch_iter = 0;

	gl_data->reconstr_data.resize ( nn->layers.back()->y.rows(), vis_interval * batch_size );
	gl_data->s_reconstr_data.resize ( nn->layers.back()->y.rows(), vis_interval * batch_size );

	while ( screen->getVisible() ) {

		while ( nn->pause ) {
			usleep ( 1000 );
			if ( nn->step || nn->quit ) { nn->step = false; break; }
		}

		// generate ( std::normal_distribution<> ( 0, 1 ), std::normal_distribution<> ( 0, 1 ),
		// 		   std::normal_distribution<> ( 0, 1 ), gan_train_data.noise.x, batch_size, INDEPENDENT );

		generate_ndims ( code_dims, std::uniform_real_distribution<> ( 0, 100 ), gan_train_data.noise.x, batch_size, INDEPENDENT );

		// generate_ndims ( code_dims, std::unifl_distribution<> ( 10, 5 ), gan_train_data.noise.x, batch_size, INDEPENDENT );
		// generate_stratified (

		//     std::uniform_real_distribution<> ( 0, 20 ),
		//     std::uniform_real_distribution<> ( 0, 20 ),
		//     std::uniform_real_distribution<> ( 0, 20 ),

		//     std::normal_distribution<> ( 0, 0.4 ),
		//     std::normal_distribution<> ( 0, 0.4 ),
		//     std::normal_distribution<> ( 0, 0.4 ),

		//     gan_train_data.noise.x, batch_size

		// );

		nn->forward ( gan_train_data.noise.x );
		gl_data->p_vertices.block ( 0, batch_size * batch_iter, 3, batch_size ) = gan_train_data.noise.x;
		gl_data->reconstr_data.block ( 0, nn->layers.back()->y.cols() * batch_iter, nn->layers.back()->y.rows(), nn->layers.back()->y.cols() ) = nn->layers.back()->y;

		//generated data
		gan_train_data.gen.x = nn->layers.back()->y;
		gan_train_data.gen.y.resize ( 1, gan_train_data.gen.x.cols() );
		gan_train_data.gen.y.setZero(); // set labels to 0 - images which came from generator

		// encoder forward pass
		encoder->forward ( gan_train_data.gen.x );
		encoder_loss = mse ( encoder->layers.back()->y, gan_train_data.noise.x ) / ( float ) batch_size;

		// encoder backward pass
		encoder->backward ( gan_train_data.noise.x - encoder->layers.back()->y );

		//real data
		gan_train_data.real.x.resize ( gan_train_data.gen.x.rows(), gan_train_data.gen.x.cols() );
		gan_train_data.real.y.resize ( 1, gan_train_data.gen.x.cols() );
		gan_train_data.training_set_indices.resize ( 1, batch_size );
		matrix_randi ( gan_train_data.training_set_indices, 0, train_data.size() - 1 );
		make_batch ( gan_train_data.real.x, train_data, gan_train_data.training_set_indices );
		gan_train_data.real.y.setOnes();

		//mix
		gan_train_data.mix_indices.resize ( 1, batch_size );
		matrix_randi ( gan_train_data.mix_indices, 0, 1 );
		gan_train_data.mixed.x = gan_train_data.gen.x;
		gan_train_data.mixed.y = gan_train_data.gen.y;
		mix ( gan_train_data.mixed.x, gan_train_data.real.x, gan_train_data.mix_indices );
		mix ( gan_train_data.mixed.y, gan_train_data.real.y, gan_train_data.mix_indices );

		// discriminator forward pass
		discriminator->forward ( gan_train_data.mixed.x );
		// encoder forward pass
		encoder->forward ( gan_train_data.mixed.x );
		gl_data->s_vertices.block ( 0, batch_size * batch_iter, 3, batch_size ) = encoder->layers.back()->y;
		gl_data->s_colors.block ( 1, batch_size * batch_iter, 1, batch_size ) = ( gan_train_data.mix_indices.cast<float>().array() ) * 0;
		gl_data->s_colors.block ( 2, batch_size * batch_iter, 1, batch_size ) = ( gan_train_data.mix_indices.cast<float>().array() ) * 1;
		gl_data->s_colors.block ( 0, batch_size * batch_iter, 1, batch_size ) = ( gan_train_data.mix_indices.cast<float>().array() ) * 0;
		gl_data->s_reconstr_data.block ( 0, encoder->layers[0]->x.cols() * batch_iter, encoder->layers[0]->x.rows(), encoder->layers[0]->y.cols() ) = gan_train_data.mixed.x;

		// loss
		gan_train_data.real_mask = gan_train_data.mix_indices.cast<float>();
		generator_loss = 0;//cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y, gan_train_data.real_mask, true );

		gan_train_data.smoothed.y = gan_train_data.mixed.y * discriminator->label_smoothing;

		discriminator->backward ( gan_train_data.smoothed.y - discriminator->layers.back()->y );

		// loss
		discriminator_real_acc = cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.smoothed.y, gan_train_data.real_mask );

		smooth_discriminator_real_acc = isNaNInf ( discriminator_real_acc ) ? smooth_discriminator_real_acc :
		                                smooth_discriminator_real_acc < 0 ? discriminator_real_acc : 0.99 *
		                                smooth_discriminator_real_acc + 0.01
		                                * discriminator_real_acc;


		// loss 2

		//invert mask
		gan_train_data.generated_mask = ( 1.0f - gan_train_data.mix_indices.cast<float>().array() );

		// gl_data->p_colors.block ( 0, batch_size * batch_iter, 1, batch_size ) = gan_train_data.real_mask;
		gl_data->p_colors.block ( 1, batch_size * batch_iter, 1, batch_size ) = gan_train_data.generated_mask.array() * 0;
		gl_data->p_colors.block ( 2, batch_size * batch_iter, 1, batch_size ) = gan_train_data.generated_mask.array() * 0;
		gl_data->p_colors.block ( 0, batch_size * batch_iter, 1, batch_size ) = gan_train_data.generated_mask.array() * discriminator->layers.back()->y.array();
		// gl_data->p_colors.block ( 3, batch_size * batch_iter, 1, batch_size ) = discriminator->layers.back()->y;

		discriminator_fake_acc = cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y, gan_train_data.generated_mask, true );

		smooth_discriminator_fake_acc = isNaNInf ( discriminator_fake_acc ) ? smooth_discriminator_fake_acc :
		                                smooth_discriminator_fake_acc < 0 ? discriminator_fake_acc : 0.99 *
		                                smooth_discriminator_fake_acc + 0.01
		                                * discriminator_fake_acc;

		// update discriminator weights
		discriminator->update ( discriminator->learning_rate, discriminator->decay );

		//discriminator cost = -1/2 Ez log D(G(z))
		if ( nn->generator_loss_type == NON_SATURATING_LOSS ) {

			// discriminator backward pass
			discriminator->backward ( 1.0f - discriminator->layers.back()->y.array() );
			discriminator->layers.back()->dy = discriminator->layers.back()->dy.array().rowwise() * gan_train_data.generated_mask.row ( 0 ).array();

			generator_loss += cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y, gan_train_data.generated_mask, false );

			// remove grads of real examples
			discriminator->layers[0]->dx = discriminator->layers[0]->dx.array().rowwise() * gan_train_data.generated_mask.row ( 0 ).array();
			nn->backward ( discriminator->layers[0]->dx.array() );

		}

		//discriminator cost = -1/2 Ez log D(1-G(z))
		if ( nn->generator_loss_type == MINIMAX_LOSS ) { //https://filebox.ece.vt.edu/~jbhuang/teaching/ece6554/sp17/lectures/cGAN-topic.pdf {

			discriminator->layers.back()->dy = discriminator->layers.back()->dy.array().rowwise() * gan_train_data.generated_mask.row ( 0 ).array();
			generator_loss += cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y, gan_train_data.generated_mask, false );

			// remove grads of real examples
			discriminator->layers[0]->dx = discriminator->layers[0]->dx.array().rowwise() * gan_train_data.generated_mask.row ( 0 ).array();
			nn->backward ( -discriminator->layers[0]->dx.array() );

		}

		// loss gen
		smooth_generator_loss = isNaNInf ( generator_loss ) ? smooth_generator_loss : smooth_generator_loss < 0 ? generator_loss : 0.99 * smooth_generator_loss + 0.01 * generator_loss;

		// update generator weights
		nn->update ( nn->learning_rate, nn->decay );

		// update encoder weights
		encoder->update ( encoder->learning_rate, encoder->decay );

		iters++;
		batch_iter++;

		// loss encoder
		smooth_encoder_loss = isNaNInf ( encoder_loss ) ? smooth_encoder_loss :
		                      smooth_encoder_loss < 0 ? encoder_loss : 0.99 * smooth_encoder_loss + 0.01
		                      * encoder_loss;


		// update graph data
		if ( iters % 100 == 0 ) {

			update_graph_data ( discriminator->loss_data, smooth_generator_loss );
			update_graph_data ( discriminator->real_acc_data, smooth_discriminator_real_acc );
			update_graph_data ( discriminator->fake_acc_data, smooth_discriminator_fake_acc );
			update_graph_data ( encoder->loss_data, smooth_encoder_loss );
			update_graph_data ( discriminator->in_grad_norm_data, discriminator->layers.back()->dy_norm() );
			update_graph_data ( nn->in_grad_norm_data, nn->layers.back()->dy_norm() );
			update_graph_data ( discriminator->out_grad_norm_data, discriminator->layers[0]->dx_norm() );
			update_graph_data ( nn->out_grad_norm_data, nn->layers[0]->dx_norm() );

			nn->collect_statistics();
			discriminator->collect_statistics();
			encoder->collect_statistics();

			update_graph_data ( discriminator->w_norm_data, discriminator->get_total_w_norm() );
			update_graph_data ( nn->w_norm_data, nn->get_total_w_norm() );
			update_graph_data ( discriminator->dw_norm_data, discriminator->get_total_dw_norm() );
			update_graph_data ( nn->dw_norm_data, nn->get_total_dw_norm() );
			update_graph_data ( discriminator->mw_norm_data, discriminator->get_total_mw_norm() );
			update_graph_data ( nn->mw_norm_data, nn->get_total_mw_norm() );

		}

		if ( iters % vis_interval == 0 ) {

			discriminator->clock = true;
			gl_data->updated();
			batch_iter = 0;

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
