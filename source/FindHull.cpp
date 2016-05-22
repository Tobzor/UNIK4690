#include "FindHull.h"
#include "SkinSegmentation.h"
// defualt constructor
FindHull::FindHull()
{
	ss = SkinSegmentation();
	cv::namedWindow("Adjust segmentation");
}

void FindHull::thresh_callback(cv::Mat background_removed, int thresh_val, bool use_otsu, bool skin_segmentation)
{
	// Blurring removed_background
	if (skin_segmentation)
	{
		Mat frame_gray;
		cv::cvtColor(background_removed, frame_gray, COLOR_BGR2GRAY);
		threshold_output = cv::Mat(frame_gray.size(), CV_8U);
		ss.skin_segmentation(background_removed,  threshold_output);

		ss.find_faces(frame_gray, faces);
		ss.delete_faces( threshold_output,  threshold_output, faces, true);

	}
	else {
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
	}

	tmp = threshold_output.clone();
	cv::cvtColor(tmp, tmp, COLOR_GRAY2BGR);
	shape_analysis(threshold_output);
}
void FindHull::shape_analysis(Mat threshold_output) {
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
	semi_approx_contour;
	approxPolyDP(Mat(contours[largest_C_index]), semi_approx_contour, 0.8, true);
	approxPolyDP(Mat(contours[largest_C_index]), approx_contour, 20, true);
	convexHull(Mat(approx_contour), approx_hull, false);
	vector<int> approx_inthull(contours[largest_C_index].size());

	convexHull(Mat(approx_contour), approx_inthull, false);
	convexityDefects(approx_contour, approx_inthull, approx_defects);

	convexHull(Mat(semi_approx_contour), semi_approx_inthull, false);
	convexityDefects(semi_approx_contour, semi_approx_inthull, semi_approx_defects);

	fingers_idx.clear();
	fingers_idx = find_finger_points(semi_approx_contour, semi_approx_inthull, semi_approx_defects);
	// Find bounding rectangle, simplified contour and max inscribed circle
	// for largest contours
	boundRect = boundingRect(contours[largest_C_index]);
	minEnclosingCircle(contours[largest_C_index], bound_circle_center, bound_circle_radius);

	find_circle();

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
	curvature = k_curvature(approx_contour, curv_below_t_idx, 1, 65 * CV_PI / 180);
	int test = 0;
	/* This loop draws every contour.
	drawContours(drawing, contours, i, colorBlue, 2, 8, vector<Vec4i>(), 0, Point());
	for (int i = 0; i < contours.size(); i++)
	{
	drawContours(drawing, hull, i, colorRed, 2, 8, vector<Vec4i>(), 0, Point());
	}
	*/
}
void FindHull::find_circle() {
	circle_radius = -1;
	float dist; Point currentPoint; int stepsize = 40;
	for (int k = 0.25*boundRect.width; k < 0.75*boundRect.width; k = k + stepsize)
	{
		for (int l = 0.25*boundRect.height; l < 0.75*boundRect.height; l = l + stepsize)
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
	Point cc = circle_center;
	for (int k = cc.x - 0.5*stepsize; k < cc.x + 0.5*stepsize; k++)
	{
		for (int l = cc.y - 0.5*stepsize; l < cc.y + 0.5*stepsize; l++)
		{
			currentPoint = Point(k, l);
			dist = pointPolygonTest(contours[largest_C_index], currentPoint, true);
			if (dist > circle_radius)
			{
				circle_radius = dist;
				circle_center = currentPoint;
			}
		}
	}

}
void FindHull::draw_contour(Mat& drawing,vector<Point> contour, Scalar color){
	vector <vector<Point>> contours;
	contours.push_back(contour);
	drawContours(drawing, contours, 0, color, 2, 8, vector<Vec4i>(), 0, Point());
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
	if (k > contour.size()) return 10000;
	if (p0_idx < 0) {
		p0_idx = p0_idx + N;
	}
	if (p2_idx > N-1) {
		p2_idx = p2_idx - N;
	}

	Point p0 = contour[p0_idx];
	Point p1 = contour[idx];
	Point p2 = contour[p2_idx];

	//cv::cvtColor(threshold_output, tmp, COLOR_GRAY2BGR);

	float angle = angle_between(p0, p1, p2);

	circle(tmp, p0, 5,Scalar(0,0,255), -1, 8);
	circle(tmp, p2, 5,Scalar(0, 0, 255), -1, 8);
	circle(tmp, p1, 5, Scalar(0, 255, 0), -1, 8);
	putText(tmp, to_string(angle*180/CV_PI), p1, CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(100, 50, 255), 1);
	imshow("K-curvatuve", tmp);

	return angle;
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

		float depth = defect[3]/256.0;
		float width = point_distance(start, stop);

		current_area = 0.5*depth*width;

		if (current_area > max_area) {
			thumb_index = defect[1];
			max_area = current_area;
		}


	}
	return thumb_index;
}


vector < int> FindHull::find_finger_points(vector <Point> contour, vector<int> hull, vector<Vec4i> defects) {
	vector <int> fingers_idx;
	int finger_idx = 0; int k = 0;
	vector <int> tmp_fingers_idx;

	for (int i = 0; i < defects.size(); i++)
	{
		Vec4i defect = defects[i];
		if (is_finger_defect(defect, contour)) {
			if (k<1) {
				tmp_fingers_idx.push_back(defect[0]);
				k++;
			}
			tmp_fingers_idx.push_back(defect[1]);
			k++;
		}
	}
	for (int i = 0; i < k; i++) {
		int idx = best_local_finger_point_idx(tmp_fingers_idx[i], contour);
		if (idx>-1) {
			fingers_idx.push_back(idx);
		}
	}
	return fingers_idx;
}
float FindHull::point_distance(Point p1, Point p2) {
	// returns the distance between two points
	Point d = p2 - p1;
	return sqrt(d.dot(d));
}
bool FindHull::is_finger_defect(Vec4i defect, vector<Point> contour) {
	int p1idx = defect[0]; int p2idx = defect[1]; int p3idx = defect[2];
	Point p1 = contour[p1idx]; 
	Point p2 = contour[p2idx];
	Point p3 = contour[p3idx];
	int depth = defect[3]/256.0;
	bool is_fd = true;

	if ((depth < 0.9*circle_radius)||(depth>bound_circle_radius)) {
		// checks that the defect is deep enough and not to deep
		// (deeper than 90% of the palm radius and less deep than
		// the radius of the least enclosing circle)
		is_fd = false;
	}
	if (angle_between(p1, p3, p2)>95 * 180 / CV_PI) {
		// checks that the defect is not wider than 95 degrees
		is_fd = false;
	}
	return is_fd;
}
int FindHull::best_local_finger_point_idx(int idx, vector <Point> contours) {
	// Checks to find the point with locally lowest k-kurvature
	// If this point have k-curvature lower than 60 degrees,
	// it is a fingerpoint
	// returns -1 if no fingerpoint is found. 
	int k = 4; 
	int best_idx = -1; int neighbor_idx;
	float current_kc;
	float lowest_kc = 10000.0f; // angles have to be larger than 360

	for (int i = -k; i <= k; i++) {
		neighbor_idx = i + idx; 
		if ((neighbor_idx >= 0) && (neighbor_idx < contours.size())) {
			current_kc = k_curvature(neighbor_idx, 4, contours) * 180 / CV_PI;
			if (current_kc < lowest_kc) {
				best_idx = neighbor_idx;
				lowest_kc = current_kc;
			}
		}
	}
	if (lowest_kc > 90) {
		// no valid fingerpoint was found
		best_idx = -1;
	}
	return best_idx;
}

float findTriangleArea(Point p1, Point p2, Point p3) {
	return 0.5*(p1.x*(p2.y - p3.y) + p2.x*(p3.y - p1.y) + p3.x*(p1.y - p2.y));
}

// deConstructor
FindHull::~FindHull()
{
	
}
