/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-23 15:52:11
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-24 19:40:26
*/

#ifndef __GLPLOTHELPER_H__
#define __GLPLOTHELPER_H__

#include <nanogui/window.h>
#include <nanogui/graph.h>

#include <iostream>
#include "perf.h"

using namespace std;

class PlotHelper : public nanogui::Window {

  public:

	PlotHelper ( Widget *parent, const std::string &title ) : nanogui::Window ( parent, title ) {

		graphs = new nanogui::Window ( this, "" );
		graphs->setFixedSize ({75, 15});
		//graphs->setLayout ( new nanogui::VGroupLayout ( 15, 0, 0, 0 ) );
		// /* FPS GRAPH */
		// graph_fps = new nanogui::Graph ( graphs, "" );
		// graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		// graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );

		graph_loss = new nanogui::Graph ( graphs, "" );
		graph_loss->setSize ({75, 15});
		graph_loss->setFooter ( "loss" );
		graph_loss->setGraphColor ( nanogui::Color ( 192, 160, 0, 255 ) );
		graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		graph_loss->values().resize(100);
		graph_loss->values().setZero();

		// //CPU graph
		// graph_cpu = new nanogui::Graph ( graphs, "" );
		// graph_cpu->values().resize ( cpu_util.size() );
		// graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		// graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );

		// // FlOP/s
		// graph_flops = new nanogui::Graph ( graphs, "" );
		// graph_flops->values().resize ( cpu_flops.size() );
		// graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		// graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );

		// // B/s
		// graph_bytes = new nanogui::Graph ( graphs, "" );
		// graph_bytes->values().resize ( cpu_reads.size() );
		// graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		// graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );


	}

	void refresh() { }

	nanogui::Window* graphs;
	nanogui::Graph *graph_loss;
	// nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;

};

#endif