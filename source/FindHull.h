#pragma once
#include "opencv2/opencv.hpp"
#include "SkinSegmentation.h"
#include "Finger.h"

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
	float k_curvature(int idx, int k, vector<Point> contour);
	bool is_finger_defect(Vec4i defect, vector<Point> contour);
	float FindHull::point_distance(Point p1, Point p2);
	Mat tmp;

	int best_local_finger_point_idx(int idx, vector <Point> contours);
	bool is_thumb_defect(Vec4i defect, vector<Point> contour);
	bool add_finger_if_valid(Finger f, vector<Finger>& fingers);
public:
	enum HandDir{LEFT=-1, RIGHT=1, UNKNOWN = 0} direction;
	float thumb_angle = -1;
	vector<vector<Point> > imgcontours;  // storing contours
	vector<vector<Point> > conv_hull;    // storing convex hull
	vector<vector<Vec4i> > conv_defects; // storing convex defects
	vector<vector<Point> > contours;

	vector<Point> approx_contour, approx_hull, semi_approx_contour;
	vector<int> semi_approx_inthull;
	vector<Vec4i> approx_defects, semi_approx_defects; // storing convex defects
	Point palm_center;
	Point2f bound_circle_center;
	float palm_radius, bound_circle_radius;
	
	Rect boundRect;
	vector<float> curvature;
	vector<int> curv_below_t_idx;
	Vec4i thumb_defect;
	vector<float> k_curvature(vector<Point> contour, vector<int>& curv_below_t_idx, int k, float threshold);
	float angle_between(Point p0, Point p1, Point p2);
	float angle_between(Point u, Point v);
	vector < int> find_finger_points(vector <Point> contour, vector<int> hull, vector<Vec4i> defects);
	vector < int> fingers_idx; 
	Point thumb_point;
	Mat drawing;
	void thresh_callback(Mat, int);
	int find_thumb();
	void draw_contour(Mat& drawing, vector<Point> contour, Scalar color);
	void find_circle();
	void shape_analysis(Mat threshold_output);

	bool skin_segmentation = false;
	bool use_otsu = false;
	bool debug_face = false;
	bool debug_thresh = false;
	bool debug_curv = false;
	Point thumb_dir;
	vector<Finger> fingers;
	//constructor
	FindHull();
	// destructor / deConstructor
	~FindHull();
};

