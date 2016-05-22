#pragma once
#include "opencv2/opencv.hpp"
#include "SkinSegmentation.h"

using namespace cv;
using namespace std;

class FindHull
{
private:
	Mat threshold_output;

	vector<Vec4i> hierarchy;

	vector<Rect > faces;
	SkinSegmentation ss;

	int largest_C_area  = 0;
	int largest_C_index = 0;
	bool is_finger_point_idx(int idx);
	float k_curvature(int idx, int k, vector<Point> contour);

public:
	vector<vector<Point> > imgcontours; // storing contours
	vector<vector<Point> > conv_hull; // storing convex hull
	vector<vector<Vec4i> > conv_defects; // storing convex defects
	vector<vector<Point> > contours;

	vector<Point> approx_contour, approx_hull, semi_approx_contour;
	vector<Vec4i> approx_defects; // storing convex defects
	Point circle_center;
	Point2f bound_circle_center;
	float circle_radius, bound_circle_radius;
	
	Rect boundRect;
	vector<float> curvature;
	vector<int> curv_below_t_idx;
	vector<float> k_curvature(vector<Point> contour, vector<int>& curv_below_t_idx, int k, float threshold);
	float angle_between(Point p0, Point p1, Point p2);
	vector < int> find_finger_points(vector <int> approx_hull_idx);
	vector < int> FindHull::find_finger_points2(vector <Point> contour);
	vector < int> fingers_idx;
	Mat drawing;
	void thresh_callback(Mat, int);
	int find_thumb();
	void draw_contour(Mat& drawing, vector<Point> contour, Scalar color);
	void find_circle();
	void shape_analysis(Mat threshold_output);

	bool skin_segmentation = true;
	bool use_otsu = true;
	bool debug = false;

	//constructor
	FindHull();
	// destructor / deConstructor
	~FindHull();
};

