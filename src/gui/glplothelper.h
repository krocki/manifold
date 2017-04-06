/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-23 15:52:11
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-25 23:37:15
*/

#ifndef __GLPLOTHELPER_H__
#define __GLPLOTHELPER_H__

#include <nanogui/window.h>
#include <nanogui/graph.h>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/entypo.h>
#include <nanogui/toolbutton.h>
#include <nanogui/checkbox.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/textbox.h>

#include <iostream>
#include "perf.h"

using namespace std;

class PlotHelper : public nanogui::Window {

  public:

	PlotHelper ( Widget *parent, const std::string &title ) : nanogui::Window ( parent, title ) {

		// root = new nanogui::Window ( this, "" );
		// root->setLayout(new nanogui::GroupLayout());

		// graphs = new nanogui::Window ( root, "" );
		// graphs->setLayout(new nanogui::GroupLayout());

		// graph_loss = new nanogui::Graph ( graphs, "" );
		// graph_loss->setFooter ( "loss" );
		// graph_loss->setGraphColor ( nanogui::Color ( 128, 128, 128, 255 ) );
		// graph_loss->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_loss->values().resize(500);
		// graph_loss->values().setZero();

		// //CPU graph
		// graph_cpu = new nanogui::Graph ( graphs, "" );
		// graph_cpu->setGraphColor ( nanogui::Color ( 192, 0, 0, 255 ) );
		// graph_cpu->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_cpu->values().resize(500);
		// graph_cpu->values().setZero();

		// // FlOP/s
		// graph_flops = new nanogui::Graph ( graphs, "" );
		// graph_flops->setGraphColor ( nanogui::Color ( 0, 192, 0, 255 ) );
		// graph_flops->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_flops->values().resize(500);
		// graph_flops->values().setZero();

		// // B/s
		// graph_bytes = new nanogui::Graph ( graphs, "" );
		// graph_bytes->setGraphColor ( nanogui::Color ( 255, 192, 0, 255 ) );
		// graph_bytes->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_bytes->values().resize(500);
		// graph_bytes->values().setZero();

		// // /* FPS GRAPH */
		// graph_fps = new nanogui::Graph ( graphs, "" );
		// graph_fps->setGraphColor ( nanogui::Color ( 0, 160, 192, 255 ) );
		// graph_fps->setBackgroundColor ( nanogui::Color ( 0, 0, 0, 32 ) );
		// graph_fps->values().resize(500);
		// graph_fps->values().setZero();

		// nanogui::Window *buttons = new nanogui::Window(root, "");
		// buttons->setPosition(Eigen::Vector2i(15, 15));
		// buttons->setLayout(new nanogui::GroupLayout());

		// nanogui::Button *b = new nanogui::Button(buttons, "save");
		// b->setCallback([] { cout << "save" << endl; });
		// b->setTooltip("save to a file");

		// b = new nanogui::Button(buttons, "load");
		// b->setCallback([] { cout << "load" << endl; });
		// b->setTooltip("load from a file");

		// nanogui::Window *popup_window = new nanogui::Window(root, "");
		// popup_window->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 16, nanogui::Alignment::Middle, 2, 2));
		// new nanogui::Label(popup_window, "Popup buttons", "sans-bold");
		// nanogui::PopupButton *popupBtn = new nanogui::PopupButton(popup_window, "Popup", ENTYPO_ICON_EXPORT);
		// nanogui::Popup *popup = popupBtn->popup();
		// popup->setLayout(new nanogui::GroupLayout());
		// new nanogui::Label(popup, "Arbitrary widgets can be placed here");
		// new nanogui::CheckBox(popup, "A check box");
		// popupBtn = new nanogui::PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
		// popup = popupBtn->popup();
		// popup->setLayout(new nanogui::GroupLayout());
		// new nanogui::CheckBox(popup, "Another check box");








		// new nanogui::Label(root, "File dialog", "sans-bold");
		// nanogui::Window *tools_window = new nanogui::Window(root, "");
		// tools_window->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
		//                         nanogui::Alignment::Middle, 0, 6));
		// b = new nanogui::Button(tools_window, "Open");
		// b->setCallback([&] {
		// 	std::cout << "File dialog result: " << nanogui::file_dialog(
		// 	{ {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << endl;
		// });
		// b = new nanogui::Button(tools_window, "Save");
		// b->setCallback([&] {
		// 	std::cout << "File dialog result: " << nanogui::file_dialog(
		// 	{ {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << endl;
		// });

		// nanogui::Window *window = new nanogui::Window(this, "Grid of small widgets");

		// window->setPosition(Eigen::Vector2i(425, 300));
		// nanogui::GridLayout *layout =
		//     new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,
		//                             nanogui::Alignment::Middle, 15, 5);
		// layout->setColAlignment(
		// { nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
		// layout->setSpacing(0, 10);
		// window->setLayout(layout);

		// /* FP widget */ {
		// 	new nanogui::Label(window, "Floating point :", "sans-bold");
		// 	nanogui::TextBox *textBox = new nanogui::TextBox(window);
		// 	textBox->setEditable(true);
		// 	textBox->setFixedSize(Eigen::Vector2i(100, 20));
		// 	textBox->setValue("50");
		// 	textBox->setUnits("GiB");
		// 	textBox->setDefaultValue("0.0");
		// 	textBox->setFontSize(16);
		// 	textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
		// }

		// /* Positive integer widget */ {
		// 	new nanogui::Label(window, "Positive integer :", "sans-bold");
		// 	auto intBox = new nanogui::IntBox<int>(window);
		// 	intBox->setEditable(true);
		// 	intBox->setFixedSize(Eigen::Vector2i(100, 20));
		// 	intBox->setValue(50);
		// 	intBox->setUnits("Mhz");
		// 	intBox->setDefaultValue("0");
		// 	intBox->setFontSize(16);
		// 	intBox->setFormat("[1-9][0-9]*");
		// 	intBox->setSpinnable(true);
		// 	intBox->setMinValue(1);
		// 	intBox->setValueIncrement(2);
		// }

		// /* Checkbox widget */ {
		// 	new nanogui::Label(window, "Checkbox :", "sans-bold");

		// 	nanogui::CheckBox* cb = new nanogui::CheckBox(window, "Check me");
		// 	cb->setFontSize(16);
		// 	cb->setChecked(true);
		// }

		// new nanogui::Label(window, "Combo box :", "sans-bold");
		// nanogui::ComboBox *cobo =
		//     new nanogui::ComboBox(window, { "Item 1", "Item 2", "Item 3" });
		// cobo->setFontSize(16);
		// cobo->setFixedSize(Eigen::Vector2i(100, 20));

		// new nanogui::Label(window, "Color button :", "sans-bold");
		// nanogui::PopupButton* popupBtn = new nanogui::PopupButton(window, "", 0);
		// popupBtn->setBackgroundColor(nanogui::Color(255, 120, 0, 255));
		// popupBtn->setFontSize(16);
		// popupBtn->setFixedSize(Eigen::Vector2i(100, 20));
		// nanogui::Popup *popup = popupBtn->popup();
		// popup->setLayout(new nanogui::GroupLayout());


		// tools_window = new nanogui::Window(root, "");
		// tools_window->setLayout(new nanogui::GroupLayout());
		// //tools_window->setWidth(600);
		// nanogui::Widget *tools = new nanogui::Widget(tools_window);
		// tools->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 16, nanogui::Alignment::Middle, 2, 2));

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLASHLIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LAYOUT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SQUARED_MINUS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SQUARED_PLUS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOCK);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DIRECTION);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_BLOCK);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_MOUSE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_COMPASS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_INSTALL);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_HAIR_CROSS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_COG);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TOOLS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TAG);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CAMERA);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_MOON);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_EYE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CLOCK);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_HOURGLASS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_GAUGE);

		// //b = new nanogui::ToolButton(tools, ENTYPO_ICON_LANGUAGE);
		// //b = new nanogui::ToolButton(tools, ENTYPO_ICON_NETWORK);

		// tools = new nanogui::Widget(tools_window);
		// tools->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 16, nanogui::Alignment::Middle, 2, 2));

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LEFT_BOLD);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DOWN_BOLD);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_UP_BOLD);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RIGHT_BOLD);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TRIANGLE_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TRIANGLE_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TRIANGLE_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TRIANGLE_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_SMALL_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_SMALL_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_SMALL_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_SMALL_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_THIN_LEFT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_THIN_DOWN );
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_THIN_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHEVRON_THIN_RIGHT);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LEFT_THIN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DOWN_THIN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_UP_THIN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RIGHT_THIN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_ARROW_COMBO);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_THREE_DOTS);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DOT);

		// tools = new nanogui::Widget(tools_window);
		// tools->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 16, nanogui::Alignment::Middle, 2, 2));

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_CASCADE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_BRANCH);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_TREE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_LINE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FLOW_PARALLEL);

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DRIVE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_ROCKET);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_KEYBOARD);

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PROGRESS_3);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PROGRESS_2);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PROGRESS_1);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PROGRESS_0);

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LIGHT_DOWN);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LIGHT_UP);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_ADJUST);

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CODE);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_MONITOR);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_INFINITY);
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LIGHT_BULB);

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DATABASE); //1F4F8
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_THERMOMETER          ); //1F4FF
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LINE_GRAPH           ); //1F4C8
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PIE_CHART            ); //25F4
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_BAR_GRAPH            ); //1F4CA
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_AREA_GRAPH           ); //1F53E
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOCK                 ); //1F512
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOCK_OPEN            ); //1F513
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOGOUT               ); //E741
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOGIN                ); //E740
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CHECK                ); //2713
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CROSS                ); //274C
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SQUARED_MINUS        ); //229F
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SQUARED_PLUS         ); //229E
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SQUARED_CROSS        ); //274E
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_MINUS        ); //2296
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_PLUS         ); //2295
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_CROSS        ); //2716
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_MINUS                ); //2796
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PLUS                 ); //2795

		// tools = new nanogui::Widget(tools_window);
		// tools->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 16, nanogui::Alignment::Middle, 2, 2));

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_HELP                 ); //2753
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CIRCLED_HELP         ); //E704		b = new nanogui::ToolButton(tools, ENTYPO_ICON_WARNING              ); //26A0
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CYCLE                ); //1F504
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CW                   ); //27F3
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_CCW                  ); //27F2
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SHUFFLE              ); //1F500
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_BACK                 ); //1F519
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LEVEL_DOWN           ); //21B3
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RETWEET              ); //E717
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LOOP                 ); //1F501
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_BACK_IN_TIME         ); //E771
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LEVEL_UP             ); //21B0
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SWITCH               ); //21C6
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_NUMBERED_LIST        ); //E005
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_ADD_TO_LIST          ); //E003
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LAYOUT               ); //268F
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_LIST                 ); //2630
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TEXT_DOC             ); //1F4C4
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TEXT_DOC_INVERTED    ); //E731

		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_UPLOAD               ); //1F4E4
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_DOWNLOAD             ); //1F4E5
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_SAVE                 ); //1F4BE
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_INSTALL              ); //E778
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PLAY                 ); //25B6
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_PAUS                 ); //2016
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RECORD               ); //25CF
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_STOP                 ); //25A0
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FF                   ); //23E9
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_FB                   ); //23EA
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TO_START             ); //23EE
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_TO_END               ); //23ED
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RESIZE_FULL          ); //E744
		// b = new nanogui::ToolButton(tools, ENTYPO_ICON_RESIZE_SMALL         ); //E746

	}

	void refresh() { }

	nanogui::Window *root;
	nanogui::Window* graphs;
	nanogui::Graph *graph_loss;
	nanogui::Graph *graph_fps, *graph_cpu, *graph_flops, *graph_bytes;

};

#endif
