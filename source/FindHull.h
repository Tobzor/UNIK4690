#pragma once
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class findHull
{
private:
	cv::Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	int largest_C_area = 0;
	int largest_C_index = 0;

public:
	vector<vector<Point> > imgcontours; // storing contours
	vector<vector<Point> > conv_hull; // storing convex hull
	vector<vector<Vec4i> > conv_defects; // storign convex defects


	cv::Mat drawing;
	void thresh_callback(cv::Mat background_removed);

	//constructor
	findHull();
	// destructor / deConstructor
	~findHull();
};

