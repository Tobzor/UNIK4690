#pragma once
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class FindHull
{
private:
	Mat threshold_output;

	vector<Vec4i> hierarchy;
	int largest_C_area = 0;
	int largest_C_index = 0;

public:
	vector<vector<Point> > imgcontours; // storing contours
	vector<vector<Point> > conv_hull; // storing convex hull
	vector<vector<Vec4i> > conv_defects; // storing convex defects
	vector<vector<Point> > contours;

	vector<Point> approx_contour, approx_hull;
	vector<Vec4i> approx_defects; // storing convex defects
	Point circle_center;
	Point2f bound_circle_center;
	float circle_radius, bound_circle_radius;
	
	Rect boundRect;


	Mat drawing;
	void thresh_callback(Mat background_removed);

	//constructor
	FindHull();
	// destructor / deConstructor
	~FindHull();
};

