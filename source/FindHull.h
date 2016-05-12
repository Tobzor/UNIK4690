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
	bool is_finger_point_idx(int idx);
	float k_curvature(int idx, int k, vector<Point> contour);

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
	vector<float> curvature;
	vector<int> curv_below_t_idx;
	vector<float> k_curvature(vector<Point> contour, vector<int>& curv_below_t_idx, int k, float threshold);
	float angle_between(Point p0, Point p1, Point p2);
	vector < int> find_finger_points(vector <int> approx_hull_idx);
	vector<int> fingers_idx;
	Mat drawing;
	void thresh_callback(Mat, int, bool);
	int find_thumb();

	//constructor
	FindHull();
	// destructor / deConstructor
	~FindHull();
};

