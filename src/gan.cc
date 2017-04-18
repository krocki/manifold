/*
* @Author: Kamil Rocki
* @Date:   2017-02-28 11:25:34
* @Last Modified by:   Kamil Rocki
* @Last Modified time: 2017-04-18 16:07:09
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

	datapoints noise, noise_n;
	datapoints gen, real, mixed;
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
int compute() {

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	
	PlotData *gl_data = screen->plot_data;
	
	// NN stuff
	double learning_rate = 1e-4f;
	float decay = 1e-6f;
	const size_t batch_size = 64;
	const int input_width = static_cast<int> ( train_data[0].x.size() );
	assert ( input_width > 0 );
	
	size_t e = 0;
	
	size_t code_dims = 3;
	
	nn = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, AE, {static_cast<int> ( code_dims ), 100, input_width }, SIGMOID ) );
	discriminator = std::shared_ptr<NN> ( new NN ( batch_size, decay, learning_rate, MLP, {input_width, 100, 1}, SIGMOID ) );
	
	nn->otype = SGD;
	nn->pause = false;
	discriminator->otype = SGD;
	discriminator->pause = true;
	
	size_t vis_interval = 1000;
	
	size_t generate_point_count = vis_interval * batch_size;
	
	generate ( std::uniform_real_distribution<> ( -40, 40 ),
			   std::uniform_real_distribution<> ( -40, 40 ),
			   std::uniform_real_distribution<> ( -40, 40 ),
			   gl_data->p_vertices, generate_point_count, INDEPENDENT );
			   
	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->p_colors, generate_point_count );
	
	generate ( std::uniform_real_distribution<> ( -40, 40 ),
			   std::uniform_real_distribution<> ( -40, 40 ),
			   std::uniform_real_distribution<> ( -40, 40 ),
			   gl_data->s_vertices, generate_point_count, INDEPENDENT );
			   
	func3::set ( {0.0f, 1.0f, 0.0f}, gl_data->s_colors, generate_point_count );
	
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
			
	}
	
	size_t iters = 0;
	float generator_loss, smooth_generator_loss = -1.0f;
	float discriminator_real_acc, smooth_discriminator_real_acc = -1.0f;
	float discriminator_fake_acc, smooth_discriminator_fake_acc = -1.0f;
	
	/* work until main window is open */
	
	int batch_iter = 0;
	
	gl_data->reconstr_data.resize ( nn->layers.back()->y.rows(), vis_interval * batch_size );
	
	while ( screen->getVisible() ) {
	
		while ( nn->pause ) {
			usleep ( 1000 );
			if ( nn->step || nn->quit ) { nn->step = false; break; }
		}
		
		// generate ( std::normal_distribution<> ( 0, 1 ), std::normal_distribution<> ( 0, 1 ),
		// 		   std::normal_distribution<> ( 0, 1 ), gan_train_data.noise.x, batch_size, INDEPENDENT );
		
		generate_ndims ( code_dims, normal_distribution<> ( 0, 10 ), gan_train_data.noise.x, batch_size, INDEPENDENT );
		
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
		
		// loss
		gan_train_data.real_mask = gan_train_data.mix_indices.cast<float>();
		generator_loss =
			0;//cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y, gan_train_data.real_mask, true );
			
		// discriminator backward pass
		discriminator->backward ( gan_train_data.mixed.y - discriminator->layers.back()->y );
		
		// loss
		discriminator_real_acc = cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y,
													  gan_train_data.real_mask );
													  
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
		
		generator_loss += cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y,
											   gan_train_data.generated_mask, false );
											   
		discriminator_fake_acc = cross_entropy_mask ( discriminator->layers.back()->y, gan_train_data.mixed.y,
													  gan_train_data.generated_mask, true );
													  
		smooth_discriminator_fake_acc = isNaNInf ( discriminator_fake_acc ) ? smooth_discriminator_fake_acc :
										smooth_discriminator_fake_acc < 0 ? discriminator_fake_acc : 0.99 *
										smooth_discriminator_fake_acc + 0.01
										* discriminator_fake_acc;
										
		// loss gen
		smooth_generator_loss = isNaNInf ( generator_loss ) ? smooth_generator_loss :
								smooth_generator_loss < 0 ? generator_loss : 0.99 * smooth_generator_loss + 0.01
								* generator_loss;
		// set generator grads
		gan_train_data.generator_dy.x = discriminator->layers[0]->dx.array().rowwise() * gan_train_data.generated_mask.row (
											0 ).array();
		nn->layers.back()->y = nn->layers.back()->y.array().rowwise() * gan_train_data.generated_mask.row ( 0 ).array();
		
		
		// generator backward pass
		nn->backward ( -gan_train_data.generator_dy.x );
		
		// update discriminator weights
		discriminator->update ( discriminator->learning_rate, discriminator->decay );
		
		// update generator weights
		nn->update ( nn->learning_rate, nn->decay );
		
		iters++;
		batch_iter++;
		
		
		// update graph data
		if ( iters % 1000 == 0 ) {
		
			update_graph_data ( discriminator->loss_data, smooth_generator_loss );
			update_graph_data ( discriminator->real_acc_data, smooth_discriminator_real_acc );
			update_graph_data ( discriminator->fake_acc_data, smooth_discriminator_fake_acc );
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
			nn->collect_statistics();
			discriminator->collect_statistics();
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
		
	}
	catch ( const std::runtime_error &e ) {
	
		std::string error_msg = std::string ( "std::runtime_error: " ) + std::string ( e.what() );
		return -1;
		
	}
	
	return 0;
}
