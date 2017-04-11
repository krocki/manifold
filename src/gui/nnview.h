/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-20 10:09:39
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-11 13:37:37
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

	NNView ( Widget *parent, NVGcontext *_nvg, PlotData *_plot_data, nanogui::Window* large_image_view, nanogui::ImageView* large_tex_view ) : nanogui::Window ( parent, "" ) {

		std::cout << "nnview init" << std::endl;
		nvg = _nvg;
		plot_data = _plot_data;
		setLayout(new nanogui::GroupLayout());
		imageView = new nanogui::ImageView ( this, _plot_data->icons[0].first );
		imageView->setVisible(false);
		popup_w = large_image_view;
		popup_i = large_tex_view;

	}

	void update_matrices() {

		// if (!initialized)

		for ( int i = 0; i < net->layers.size(); i++ ) {

			imgPanel[i]->setImages ( plot_data->nn_matrices[i] );
			Eigen::Vector2i grid = Eigen::Vector2i(2, 4);
			imgPanel[i]->setFixedGrid(grid);

			imgPanel[i]->setCallback ( [this, i] ( int k ) {

				std::cout << "Selected item " << i << ", " << k << '\n';
				if (popup_w && popup_i) {
					popup_i->bindImage(plot_data->nn_matrices[i][k].first);
					popup_w->setVisible(true);
					popup_w->requestFocus();
				}


			});

		}

		performLayout ( nvg );

		// initialized = true;
	}

	void setnet ( std::shared_ptr<NN> _net ) {

		net = _net;

		if ( net ) {

			wwww =  new nanogui::Window ( this, "" );
			wwww->setLayout( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, net->layers.size(), nanogui::Alignment::Middle,
			                 3, 3 ));


			// labels
			for ( int i = 0; i < net->layers.size(); i++ ) {

				nanogui::Label* l = new nanogui::Label ( wwww, std::to_string ( i ) + ": " + net->layers[i]->name + ", x: "  + 								std::to_string ( net->layers[i]->x.rows() ) + ":" +  std::to_string ( net->layers[i]->x.cols() ) + ", y: "  +								std::to_string ( net->layers[i]->y.rows() ) + ":" + std::to_string ( net->layers[i]->y.cols() ), "sans-bold", 9);
				l->setFixedSize({ ( ( size() [0] - 30 )  / (net->layers.size()) ) - 10, 20});

			}



			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout(new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                                        0, 0 ));
				avg_inputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_inputs[i]->setFooter ( "avg x" );
				avg_inputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				avg_inputs[i]->values_ptr()->resize ( 500 );
				avg_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_avg_activity = avg_inputs[i]->values_ptr();

				hist_inputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_inputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				hist_inputs[i]->values_ptr()->resize ( 10 );
				hist_inputs[i]->values_ptr()->setZero();
				net->layers[i]->x_hist_activity = hist_inputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout(new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                                        0, 0 ));
				avg_outputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_outputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_outputs[i]->setFooter ( "avg y" );
				avg_outputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				avg_outputs[i]->values_ptr()->resize ( 500 );
				avg_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_avg_activity = avg_outputs[i]->values_ptr();

				hist_outputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_outputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_outputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				hist_outputs[i]->values_ptr()->resize ( 10 );
				hist_outputs[i]->values_ptr()->setZero();
				net->layers[i]->y_hist_activity = hist_outputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout(new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                                        0, 0 ));
				avg_dinputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				avg_dinputs[i]->setFooter ( "avg dx" );
				avg_dinputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				avg_dinputs[i]->values_ptr()->resize ( 500 );
				avg_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_avg_activity = avg_dinputs[i]->values_ptr();

				hist_dinputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_dinputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_dinputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_inputs[i]->setFooter ( "hist x" );
				hist_dinputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				hist_dinputs[i]->values_ptr()->resize ( 10 );
				hist_dinputs[i]->values_ptr()->setZero();
				net->layers[i]->dx_hist_activity = hist_dinputs[i]->values_ptr();


			}

			for ( int i = 0; i < net->layers.size(); i++ ) {
				nanogui::Widget *gw = new nanogui::Widget ( wwww );
				gw->setLayout(new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle,
				                                        0, 0 ));
				avg_doutputs.push_back ( new nanogui::Graph ( gw, "" ) );
				avg_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				avg_doutputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
				avg_doutputs[i]->setFooter ( "avg dy" );
				avg_doutputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				avg_doutputs[i]->values_ptr()->resize ( 500 );
				avg_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_avg_activity = avg_doutputs[i]->values_ptr();

				hist_doutputs.push_back ( new nanogui::Graph ( gw, "", nanogui::GraphType::GRAPH_COLORBARS ) );
				hist_doutputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
				hist_doutputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
				// hist_outputs[i]->setFooter ( "hist x" );
				hist_doutputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / (net->layers.size() * 2) ) - 10, 30} );
				hist_doutputs[i]->values_ptr()->resize ( 10 );
				hist_doutputs[i]->values_ptr()->setZero();
				net->layers[i]->dy_hist_activity = hist_doutputs[i]->values_ptr();

			}

			for ( int i = 0; i < net->layers.size(); i++ ) {

				iw.push_back(new nanogui::Widget ( wwww ));
				iw[i]->setLayout ( new nanogui::BoxLayout() );
				iw[i]->setFixedSize({ ( ( size() [0] - 30 )  / (net->layers.size()) ) - 10, 300});
				imgPanel.push_back(new nanogui::ImagePanel ( iw[i], ( ( size() [0] - 50 )  / (net->layers.size() * 2) ) - 10, 3, 3 ));

			}

			// nanogui::VScrollPanel *vscroll = new nanogui::VScrollPanel ( www );
			// nanogui::Window *ww = new nanogui::Window ( vscroll, "" );
			// ww->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 8, nanogui::Alignment::Middle, 15, 5 ) );

			// 	imgPanel->setCallback ( [this, imgPanel] ( int i ) {

			// 		imageView->setVisible(true);
			// 		// Change the active textures.
			// 		imgPanel->setCallback ( [this, imgPanel] ( int i ) {
			// 			imageView->bindImage ( plot_data->icons[i].first );
			// 			std::cout << "Selected item " << i << '\n';
			// 		} );

			// 		imageView->setGridThreshold ( 20 );
			// 		imageView->setPixelInfoThreshold ( 20 );
			// 		imageView->setPixelInfoCallback (
			// 		[this, i] ( const Eigen::Vector2i & index ) -> std::pair<std::string, nanogui::Color> {
			// 			auto &imageData = plot_data->icons[i].second;
			// 			auto &textureSize = imageView->imageSize();
			// 			std::string stringData;
			// 			uint16_t channelSum = 0;
			// 			for ( int k = 0; k != 4; ++k ) {
			// 				auto &channelData = imageData[4 * index.y() * textureSize.x() + 4 * index.x() + k];
			// 				channelSum += channelData;
			// 				stringData += ( std::to_string ( static_cast<int> ( channelData ) ) + "\n" );
			// 			}
			// 			float intensity = static_cast<float> ( 255 - ( channelSum / 4 ) ) / 255.0f;
			// 			float colorScale = intensity > 0.5f ? ( intensity + 1 ) / 2 : intensity / 2;
			// 			nanogui::Color textColor = nanogui::Color ( colorScale, 1.0f );
			// 			return { stringData, textColor };
			// 		} );

			// 		this->performLayout ( nvg );

			// 	} );

			// }

			performLayout ( nvg );
		}
		// 		nanogui::Window *lay =  root->add<nanogui::Window> ( std::to_string ( i ) + ": " + net->layers[i]->name + ", x: "  +
		// 								std::to_string ( net->layers[i]->x.rows() ) + ":" +  std::to_string ( net->layers[i]->x.cols() ) + ", y: "  +
		// 								std::to_string ( net->layers[i]->y.rows() ) + ":" +
		// 								std::to_string ( net->layers[i]->y.cols() ) );
		// 		layers_windows.push_back ( lay );
		// 		layers_windows[i]->setFixedSize ( { ( ( size() [0] - 60 )  / net->layers.size() ), 200} );
		// 		layers_windows[i]->setLayout ( new nanogui::GridLayout ( nanogui::Orientation::Horizontal, 1,
		// 									   nanogui::Alignment::Middle,
		// 									   5, 5 ) ) ;

		// 		avg_inputs.push_back ( new nanogui::Graph ( layers_windows[i], "" ) );
		// 		avg_inputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
		// 		avg_inputs[i]->setGraphColor ( nanogui::Color ( 255, 255, 255, 128 ) );
		// 		avg_inputs[i]->setFooter ( "avg x" );

		// 		avg_outputs.push_back ( new nanogui::Graph ( layers_windows[i], "" ) );
		// 		avg_outputs[i]->setBackgroundColor ( nanogui::Color ( 32, 32, 32, 128 ) );
		// 		avg_outputs[i]->setGraphColor ( nanogui::Color ( 192, 128, 0, 128 ) );
		// 		avg_outputs[i]->setFooter ( "avg y" );

		// 		avg_outputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / net->layers.size() ) - 10, 40} );
		// 		avg_outputs[i]->values_ptr()->resize ( 500 );
		// 		avg_outputs[i]->values_ptr()->setZero();

		// 		avg_inputs[i]->setFixedSize ( { ( ( size() [0] - 60 )  / net->layers.size() ) - 10, 40} );
		// 		avg_inputs[i]->values_ptr()->resize ( 500 );
		// 		avg_inputs[i]->values_ptr()->setZero();

		// 		net->layers[i]->x_avg_activity = avg_inputs[i]->values_ptr();
		// 		net->layers[i]->y_avg_activity = avg_outputs[i]->values_ptr();

		// 		performLayout ( nvg );
		// 	}

		// }
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
	nanogui::ImageView* imageView;
	nanogui::Window *wwww;
	int selected = -1;

	std::shared_ptr<NN> net;
	nanogui::Window* popup_w;
	nanogui::ImageView* popup_i;
	std::vector<nanogui::Window *> layers_windows;
	std::vector<nanogui::Widget *> iw;
	std::vector<nanogui::ImagePanel *> imgPanel;
	std::vector<nanogui::Graph *> avg_outputs, avg_inputs, hist_outputs, hist_inputs;
	std::vector<nanogui::Graph *> avg_doutputs, avg_dinputs, hist_doutputs, hist_dinputs;

	NVGcontext *nvg = nullptr;
	PlotData *plot_data = nullptr;
};

#endif