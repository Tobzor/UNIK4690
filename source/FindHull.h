#pragma once
#include "opencv2/opencv.hpp"
class findHull
{
private:
	cv::Mat threshold_output;


public:
	cv::Mat drawing;
	void thresh_callback(cv::Mat background_removed);

	//constructor
	findHull();
	// destructor / deConstructor
	~findHull();
};

