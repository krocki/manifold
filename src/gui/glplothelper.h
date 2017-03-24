/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-03-23 15:52:11
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-23 15:53:30
*/

#ifndef __GLPLOTHELPER_H__
#define __GLPLOTHELPER_H__

#include <nanogui/window.h>

#include <iostream>

using namespace std;

class PlotHelper : public nanogui::Window {

  public:

	PlotHelper ( Widget *parent, const std::string &title ) : nanogui::Window ( parent, title ) {



	}


};

#endif