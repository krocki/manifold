/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:09:39
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-12 20:26:44
*/

#ifndef __NNVIEW_H__
#define __NNVIEW_H__

#include <nanogui/textbox.h>
#include <nanogui/imageview.h>
#include <nanogui/messagedialog.h>
#include <nanogui/popup.h>
#include <nanogui/popupbutton.h>

class NNView : public nanogui::Window {

  public:

	NNView ( Widget *parent, NVGcontext *_nvg, PlotData *_plot_data, nanogui::Window *large_image_view,
	         nanogui::ImageView *large_tex_view ) : nanogui::Window ( parent, "" ) {

		std::cout << "nnview init" << std::endl;
		nvg = _nvg;
		plot_data = _plot_data;
		setLayout ( new nanogui::GroupLayout() );
		popup_w = large_image_view;
		popup_i = large_tex_view;

	}

	void update_matrices() {

		// if (!initialized)

		for ( int i = 0; i < net->layers.size(); i++ ) {

			imgPanel[i]->setImages ( plot_data->nn_matrices[i] );
			imgPanel[i]->setCallback ( [this, i] ( int k ) {

				std::cout << "Selected item " << i << ", " << k << '\n';
				if ( popup_w && popup_i ) {

					if ( last_selected != plot_data->nn_matrices[i][k].first || !popup_w->visible() ) {

						popup_i->bindImage ( plot_data->nn_matrices[i][k].first );
						popup_w->setVisible ( true );
						popup_w->requestFocus();
						last_selected = plot_data->nn_matrices[i][k].first;

					} else {

						popup_w->setVisible ( false );
						last_selected = -1;

					}


				}
			} );

		}

		if ( disc ) {
			for ( int i = 0; i < disc->layers.size(); i++ ) {

				imgPanel_disc[i]->setImages ( plot_data->disc_matrices[i] );
				imgPanel_disc[i]->setCallback ( [this, i] ( int k ) {

					std::cout << "Selected item " << i << ", " << k << '\n';
					if ( popup_w && popup_i ) {

						if ( last_selected != plot_data->disc_matrices[i][k].first || !popup_w->visible() ) {

							popup_i->bindImage ( plot_data->disc_matrices[i][k].first );
							popup_w->setVisible ( true );
							popup_w->requestFocus();
							last_selected = plot_data->disc_matrices[i][k].first;

						} else {

							popup_w->setVisible ( false );
							last_selected = -1;

						}


					}
				} );

			}
		}

		std::vector<std::pair<int, std::string>> dummy_vector;
		dummy_vector.resize ( 4 );
		dummy_vector[0] = std::make_pair ( plot_data->gan_generator_data_textures.id, "generator data" );
		dummy_vector[1] = std::make_pair ( plot_data->gan_real_data_textures.id, "real images" );
		dummy_vector[2] = std::make_pair ( plot_data->gan_mixed_data_textures.id, "mixed images" );
		dummy_vector[3] = std::make_pair ( plot_data->gan_generator_dy_textures.id, "generator dy" );

		gan_data_panel->setImages ( dummy_vector );

		performLayout ( nvg );

		// initialized = true;
	}

	void setnet ( std::shared_ptr<NN> _net ) {

		net = _net;

		if ( net ) {

			wwww =  new nanogui::Window ( this, "" );
			wwww->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, net->layers.size(),
			                  nanogui::Alignment::Middle,
			                  3, 3 ) );

			// labels
			for ( int i = 0; i < net->layers.size(); i++ ) {

				nanogui::Label *l = new nanogui::Label ( wwww,
				        std::to_string ( i ) + ": " + net->layers[i]->name + ", x: "  + 	std::to_string ( net->layers[i]->x.rows() ) + ":" +
				        std::to_string ( net->layers[i]->x.cols() ) + ", y: "  + std::to_string ( net->layers[i]->y.rows() ) + ":" +
				        std::to_string ( net->layers[i]->y.cols() ), "sans-bold", 9 );
				l->setFixedSize ( { ( ( size() [0] - 30 )  / ( net->layers.size() ) ) - 10, 20} );

			}



			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_inputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_inputs[i]->setFooter ( "avg x" );
				avg_inputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				avg_inputs[i]->values_ptr()->resize ( 500 );
				avg_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_avg_activity = avg_inputs[i]->values_ptr();

				hist_inputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_inputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				hist_inputs[i]->values_ptr()->resize ( 10 );
				hist_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_hist_activity = hist_inputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_outputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_outputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_outputs[i]->setFooter ( "avg y" );
				avg_outputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				avg_outputs[i]->values_ptr()->resize ( 500 );
				avg_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_avg_activity = avg_outputs[i]->values_ptr();

				hist_outputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_outputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_outputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				hist_outputs[i]->values_ptr()->resize ( 10 );
				hist_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_hist_activity = hist_outputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_dinputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_dinputs[i]->setFooter ( "avg dx" );
				avg_dinputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				avg_dinputs[i]->values_ptr()->resize ( 500 );
				avg_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_avg_activity = avg_dinputs[i]->values_ptr();

				hist_dinputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_dinputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				hist_dinputs[i]->values_ptr()->resize ( 10 );
				hist_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_hist_activity = hist_dinputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_doutputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_doutputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_doutputs[i]->setFooter ( "avg dy" );
				avg_doutputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				avg_doutputs[i]->values_ptr()->resize ( 500 );
				avg_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_avg_activity = avg_doutputs[i]->values_ptr();

				hist_doutputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_doutputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_doutputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / ( net->layers.size() * 2 ) ) - 10, 30} );
				hist_doutputs[i]->values_ptr()->resize ( 10 );
				hist_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_hist_activity = hist_doutputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {

				iw.push_back ( new nanogui::Widget ( wwww ) );
				iw[i]->setLayout ( new nanogui::BoxLayout() );
				iw[i]->setFixedSize ( { ( ( size() [0] - 30 )  / ( net->layers.size() ) ) - 10, 300} );
				imgPanel.push_back ( new nanogui::ImagePanel ( iw[i], ( ( size() [0] - 50 )  / ( net->layers.size() * 2 ) ) - 10, 3,
				                     3 ) );

			}

			performLayout ( nvg );
			setVisible ( false );
		}
	}

	void setnets ( std::shared_ptr<NN> _net, std::shared_ptr<NN> _disc ) {

		net = _net;
		disc = _disc;

		nanogui::Window *nets =	new nanogui::Window ( this, "" );
		// nets->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2 ) );

		// nanogui::GridLayout *outputslayout = new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 10,
		//         nanogui::Alignment::Middle, 3, 3 );
		// outputslayout->setColAlignment ( { nanogui::Alignment::Maximum, nanogui::Alignment::Fill } );
		// outputslayout->setSpacing ( 0, 3 );

		if ( net ) {

			wwww =  new nanogui::Window ( nets, "" );
			wwww->setPosition ( {5, 5} );
			wwww->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, net->layers.size(),
			                  nanogui::Alignment::Middle ) );

			// labels
			for ( int i = 0; i < net->layers.size(); i++ ) {

				nanogui::Label *l = new nanogui::Label ( wwww,
				        std::to_string ( i ) + ": " + net->layers[i]->name + ", x: "  + std::to_string ( net->layers[i]->x.rows() ) + ":" +
				        std::to_string ( net->layers[i]->x.cols() ) + ", y: "  + std::to_string ( net->layers[i]->y.rows() ) + ":" +
				        std::to_string ( net->layers[i]->y.cols() ), "sans-bold", 9 );
				l->setFixedSize ( { 40, 30} );
			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_inputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_inputs[i]->setFooter ( "avg x" );
				avg_inputs[i]->setFixedSize ( {45, 45} );
				avg_inputs[i]->values_ptr()->resize ( 500 );
				avg_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_avg_activity = avg_inputs[i]->values_ptr();

				hist_inputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_inputs[i]->setFixedSize ( { 45, 45} );
				hist_inputs[i]->values_ptr()->resize ( 10 );
				hist_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_hist_activity = hist_inputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_outputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_outputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_outputs[i]->setFooter ( "avg y" );
				avg_outputs[i]->setFixedSize ( {45, 45} );
				avg_outputs[i]->values_ptr()->resize ( 500 );
				avg_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_avg_activity = avg_outputs[i]->values_ptr();

				hist_outputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_outputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_outputs[i]->setFixedSize ( { 45, 45} );
				hist_outputs[i]->values_ptr()->resize ( 10 );
				hist_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_hist_activity = hist_outputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_dinputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_dinputs[i]->setFooter ( "avg dx" );
				avg_dinputs[i]->setFixedSize ( {45, 45} );
				avg_dinputs[i]->values_ptr()->resize ( 500 );
				avg_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_avg_activity = avg_dinputs[i]->values_ptr();

				hist_dinputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_dinputs[i]->setFixedSize ( {45, 45} );
				hist_dinputs[i]->values_ptr()->resize ( 10 );
				hist_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_hist_activity = hist_dinputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_doutputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_doutputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_doutputs[i]->setFooter ( "avg dy" );
				avg_doutputs[i]->setFixedSize ( {45, 45} );
				avg_doutputs[i]->values_ptr()->resize ( 500 );
				avg_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_avg_activity = avg_doutputs[i]->values_ptr();

				hist_doutputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_doutputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_doutputs[i]->setFixedSize ( {45, 45} );
				hist_doutputs[i]->values_ptr()->resize ( 10 );
				hist_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_hist_activity = hist_doutputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {

				nanogui::Window *iww = new nanogui::Window ( wwww, "" );
				imgPanel.push_back ( new nanogui::ImagePanel ( iww, 40, 3, 3, {2, 3} ) );
				Eigen::Vector2i w_size = imgPanel.back()->preferredSize() + Eigen::Vector2i ( {0, 3} );
				iww->setSize ( w_size );
			}
		}

		nanogui::Window *gan_data = new nanogui::Window ( nets, "" );
		gan_data->setLayout ( new nanogui::GroupLayout() );
		gan_data->setFixedSize ( {650, 650} );
		gan_data->setPosition ( {600, 50} );
		gan_data_panel = new nanogui::ImagePanel ( gan_data, 300, 3, 3, {2, 2} );

		if ( disc ) {

			wwww =  new nanogui::Window ( nets, "" );
			wwww->setPosition ( {50, 400} );
			wwww->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, disc->layers.size(),
			                  nanogui::Alignment::Middle,
			                  3, 3 ) );


			// labels
			for ( int i = 0; i < disc->layers.size(); i++ ) {

				nanogui::Label *l = new nanogui::Label ( wwww,
				        std::to_string ( i ) + ": " + disc->layers[i]->name + ", x: "  + std::to_string ( disc->layers[i]->x.rows() ) + ":" +
				        std::to_string ( disc->layers[i]->x.cols() ) + ", y: "  +	std::to_string ( disc->layers[i]->y.rows() ) + ":" +
				        std::to_string ( disc->layers[i]->y.cols() ), "sans-bold", 9 );
				l->setFixedSize ( { 40, 30} );

			}



			for ( int i = 0; i < disc->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_inputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_inputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_inputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_inputs.back()->setFooter ( "avg x" );
				avg_inputs.back()->setFixedSize ( {47, 30} );
				avg_inputs.back()->values_ptr()->resize ( 500 );
				avg_inputs.back()->values_ptr()->setZero();
				disc->layers[i]->x_avg_activity = avg_inputs.back()->values_ptr();

				hist_inputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_inputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_inputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_inputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				hist_inputs.back()->values_ptr()->resize ( 10 );
				hist_inputs.back()->values_ptr()->setZero();
				disc->layers.back()->x_hist_activity = hist_inputs.back()->values_ptr();


			}

			for ( int i = 0; i < disc->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_outputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_outputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_outputs.back()->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_outputs.back()->setFooter ( "avg y" );
				avg_outputs.back()->setFixedSize ( {47, 30} );
				avg_outputs.back()->values_ptr()->resize ( 500 );
				avg_outputs.back()->values_ptr()->setZero();
				disc->layers[i]->y_avg_activity = avg_outputs.back()->values_ptr();

				hist_outputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_outputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_outputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_outputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				hist_outputs.back()->values_ptr()->resize ( 10 );
				hist_outputs.back()->values_ptr()->setZero();
				disc->layers[i]->y_hist_activity = hist_outputs.back()->values_ptr();

			}

			for ( int i = 0; i < disc->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_dinputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_dinputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_dinputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_dinputs.back()->setFooter ( "avg dx" );
				avg_dinputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				avg_dinputs.back()->values_ptr()->resize ( 500 );
				avg_dinputs.back()->values_ptr()->setZero();
				disc->layers[i]->dx_avg_activity = avg_dinputs.back()->values_ptr();

				hist_dinputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_dinputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_dinputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_dinputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				hist_dinputs.back()->values_ptr()->resize ( 10 );
				hist_dinputs.back()->values_ptr()->setZero();
				disc->layers[i]->dx_hist_activity = hist_dinputs.back()->values_ptr();


			}

			for ( int i = 0; i < disc->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                0, 0 ) );
				avg_doutputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_doutputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_doutputs.back()->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_doutputs.back()->setFooter ( "avg dy" );
				avg_doutputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				avg_doutputs.back()->values_ptr()->resize ( 500 );
				avg_doutputs.back()->values_ptr()->setZero();
				disc->layers[i]->dy_avg_activity = avg_doutputs.back()->values_ptr();

				hist_doutputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_doutputs.back()->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_doutputs.back()->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_doutputs.back()->setFixedSize ( { ( ( size() [0] - 60 )  / ( disc->layers.size() * 4 ) ) - 30, 30} );
				hist_doutputs.back()->values_ptr()->resize ( 10 );
				hist_doutputs.back()->values_ptr()->setZero();
				disc->layers[i]->dy_hist_activity = hist_doutputs.back()->values_ptr();

			}

			for ( int i = 0; i < disc->layers.size(); i++ ) {

				nanogui::Window *iww = new nanogui::Window ( wwww, "" );
				imgPanel_disc.push_back ( new nanogui::ImagePanel ( iww, 40, 3, 3, {2, 3} ) );
				Eigen::Vector2i w_size = imgPanel.back()->preferredSize() + Eigen::Vector2i ( {0, 3} );
				iww->setSize ( w_size );
			}

		}

		performLayout ( nvg );
		setVisible ( true );

	}

	virtual bool resizeEvent ( const Eigen::Vector2i &size ) {

		// setSize ( size );
		// root->setSize ( size );

		// for ( int i = 0; i < ( int ) layers_windows.size(); i++ ) {
		// 	layers_windows[i]->setSize ( { ( ( size [0] - 100 )  / net->layers.size() ), 200} );
		// }

		// performLayout ( nvg );

		return true;

	}

	// virtual void draw ( NVGcontext *ctx ) {

	// 	if ( net ) {



	// 	}

	// }

	bool initialized = false;
	nanogui::Widget *parent;
	nanogui::ImageView *imageView;
	nanogui::Window *wwww;
	int last_selected = -1;

	std::shared_ptr<NN> net;
	std::shared_ptr<NN> disc;
	nanogui::Window *popup_w;
	nanogui::ImageView *popup_i;
	std::vector<nanogui::Window *> layers_windows;
	std::vector<nanogui::Widget *> iw;
	std::vector<nanogui::ImagePanel *> imgPanel;
	nanogui::ImagePanel *gan_data_panel;
	std::vector<nanogui::ImagePanel *> imgPanel_disc;
	std::vector<nanogui::Graph *> avg_outputs, avg_inputs, hist_outputs, hist_inputs;
	std::vector<nanogui::Graph *> avg_doutputs, avg_dinputs, hist_doutputs, hist_dinputs;

	NVGcontext *nvg = nullptr;
	PlotData *plot_data = nullptr;
};

#endif