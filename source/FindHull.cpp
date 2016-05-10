#include "FindHull.h"

// defualt constructor
FindHull::FindHull()
{
}

void FindHull::thresh_callback(cv::Mat background_removed, int thresh_val, bool use_otsu)
{
	// Blurring removed_background
	blur(background_removed, background_removed, Size(3, 3));

	if (!use_otsu) {
		// Detect edges using Threshold, with trackbar values.
		threshold(background_removed, threshold_output, thresh_val, 255, THRESH_BINARY);
	}
	else
	{
		// Detect edges using Threshold
		threshold(background_removed, threshold_output, 0, 255, THRESH_OTSU);
	}
	
	imshow("Threshold", threshold_output);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));


	if (contours.size() <= 0) return;
	/// Find the convex hull object for each contour
	vector<vector<Point> >hull(contours.size());
	vector<vector<Vec4i> >defects(contours.size());
	vector<vector<int> >inthull(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		convexHull(Mat(contours[i]), hull[i], false);

		// This is for finding defects - something is wrong though.
		convexHull(Mat(contours[i]), inthull[i], false);
		if (inthull[i].size() > 3) {
			convexityDefects(contours[i], inthull[i], defects[i]);
		}
	}

	// Save contours, hull and defects in public variables.
	// Can now access from objects --> drawing class?
	conv_hull = hull;
	conv_defects = defects;
	// imgcontours = contours; not necessary when contours is public.

	// resetting values because genius.
	largest_C_area = 0; largest_C_index = 0;
	// Iterate through contours and find largest one
	for (int i = 0; i < contours.size(); i++)
	{
		// Find area of current contour
		double area = contourArea(contours[i], false);
		if (area > largest_C_area)
		{
			largest_C_area = area;
			// Storing the index
			largest_C_index = i;
		}
	} // end of largest contour search
	approx_contour.resize(contours[largest_C_index].size());
	approxPolyDP(Mat(contours[largest_C_index]), approx_contour, 20, true);
	convexHull(Mat(approx_contour), approx_hull, false);
	vector<int> approx_inthull(contours[largest_C_index].size());

	convexHull(Mat(approx_contour), approx_inthull, false);
	convexityDefects(approx_contour, approx_inthull, approx_defects);
	fingers_idx.clear();
	fingers_idx = find_finger_points(approx_inthull);
	// Find bounding rectangle, simplified contour and max inscribed circle
	// for largest contours
	boundRect = boundingRect(contours[largest_C_index]);
	minEnclosingCircle(contours[largest_C_index], bound_circle_center, bound_circle_radius);

	float dist;
	circle_radius = -1;
	Point currentPoint;
	int stepsize = 20;
	for (int k = 0; k < boundRect.width; k = k + stepsize)
	{
		for (int l = 0; l < boundRect.height; l = l + stepsize)
		{
			currentPoint = Point(boundRect.x + k, boundRect.y + l);
			dist = pointPolygonTest(contours[largest_C_index], currentPoint, true);
			if (dist > circle_radius)
			{
				circle_radius = dist;
				circle_center = currentPoint;
			}
		}
	}

	RNG rng(12345);
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	// Scalar colorBlue = Scalar(255, 0, 0);
	// Scalar colorRed = Scalar(0, 0, 255);

	// Draw contours + hull results in this mat.
	drawing = Mat::zeros(threshold_output.size(), CV_8UC3);

	// Checks if there are contours, then draws these into drawing
	if (contours.size() > 0) {
		drawContours(drawing, contours, largest_C_index, color, 2, 8, vector<Vec4i>(), 0, Point());
		drawContours(drawing, hull, largest_C_index, color, 2, 8, vector<Vec4i>(), 0, Point());
	}
	
	curv_below_t_idx.clear();
	curvature = k_curvature(approx_contour, curv_below_t_idx,1,  65*CV_PI/180);
	int test = 0;
	/* This loop draws every contour.
		drawContours(drawing, contours, i, colorBlue, 2, 8, vector<Vec4i>(), 0, Point());
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(drawing, hull, i, colorRed, 2, 8, vector<Vec4i>(), 0, Point());
	}
	*/
}
vector<float> FindHull::k_curvature(vector<Point> contour, vector<int>& curv_below_t_idx, int k, float threshold)
{
	vector<float> curvature = vector<float>(contour.size());
	//vector<int> curv_below_t_idx;
	for (int i = k; i < contour.size()-k; i++) {
		Point p0 = contour[i - k];
		Point p1 = contour[i];
		Point p2 = contour[i + k];
		curvature[i] = angle_between(p0, p1, p2);
		if (curvature[i] < threshold){
			curv_below_t_idx.push_back(i);
		}
	}
	return curvature;
}

float FindHull::k_curvature(int idx,int k, vector<Point> contour) {
	
	int p0_idx = idx - k;
	int p2_idx = idx + k;

	int N = contour.size();

	if (p0_idx < 0) {
		p0_idx = p0_idx + N;
	}
	if (p2_idx > N-1) {
		p2_idx = p2_idx - N;
	}
	
	Point p0 = contour[p0_idx];
	Point p1 = contour[idx];
	Point p2 = contour[p2_idx];
	return angle_between(p0, p1, p2);
}

float FindHull::angle_between(Point p0, Point p1, Point p2) {
	Point v = p0 - p1; Point u = p2 - p1;

	float dot_product = v.dot(u);
	float length_v = sqrt(v.dot(v));
	float length_u = sqrt(u.dot(u));
	return acos(dot_product / (length_u*length_v));

}

int FindHull::find_thumb() {
	float current_area;
	float max_area = -1;
	int thumb_index = 0;
	for (int i = 0; i < approx_defects.size(); i++)
	{
		Vec4i defect = approx_defects[i];
		Point start  = approx_contour[defect[0]];
		Point stop   = approx_contour[defect[1]];
		Point d = start - stop;

		float depth = defect[3]/256.0;
		float width = sqrt((d).dot(d));

		current_area = 0.5*depth*width;

		if (current_area > max_area) {
			thumb_index = defect[0];
			max_area = current_area;
		}


	}
	return thumb_index;
}

vector < int> FindHull::find_finger_points(vector <int> approx_hull_idx) {
	vector <int> fingers_idx;
	int finger_idx = 0;
	int k = 1;
	for (int i = k; i < approx_hull_idx.size()-k; i++)
	{
		finger_idx = approx_hull_idx[i];
		if (is_finger_point_idx(finger_idx)) {
			fingers_idx.push_back(finger_idx);
		}
	}
	return fingers_idx;
}

bool FindHull::is_finger_point_idx(int idx) {
	bool is_fp_idx = true;
	float kc = k_curvature(idx, 1, approx_contour)*180/CV_PI;
	if ( kc > 75) {
		is_fp_idx = false;
	}
	else {
		int test = 0;
	}
	return is_fp_idx;
}

float findTriangleArea(Point p1, Point p2, Point p3) {
	return 0.5*(p1.x*(p2.y - p3.y) + p2.x*(p3.y - p1.y) + p3.x*(p1.y - p2.y));
}

// deConstructor
FindHull::~FindHull()
{
	
}
