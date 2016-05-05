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
		threshold(background_removed, threshold_output, thresh_val, 255, THRESH_BINARY_INV);
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
	approxPolyDP(Mat(contours[largest_C_index]), approx_contour, 30, true);
	convexHull(Mat(approx_contour), approx_hull, false);
	vector<int> approx_inthull(contours[largest_C_index].size());

	convexHull(Mat(approx_contour), approx_inthull, false);
	convexityDefects(approx_contour, approx_inthull, approx_defects);

	// Find bounding rectangle, simplified contour and max inscribed circle
	// for largest contours
	boundRect   = boundingRect(contours[largest_C_index]);
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
	//Scalar colorBlue = Scalar(255, 0, 0);
	//Scalar colorRed = Scalar(0, 0, 255);

	// Draw contours + hull results in this mat.
	drawing = Mat::zeros(threshold_output.size(), CV_8UC3);

	// Checks if there are contours, then draws these into drawing
	if (contours.size() > 0) {
		drawContours(drawing, contours, largest_C_index, color, 2, 8, vector<Vec4i>(), 0, Point());
		drawContours(drawing, hull, largest_C_index, color, 2, 8, vector<Vec4i>(), 0, Point());
	}

	/* This loop draws every contour.
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(drawing, contours, i, colorBlue, 2, 8, vector<Vec4i>(), 0, Point());
		drawContours(drawing, hull, i, colorRed, 2, 8, vector<Vec4i>(), 0, Point());
	}
	*/
}
vector<float> FindHull::k_curvature(vector<Point> contour, vector<int>& curv_below_t_idx, int k, float threshold)
{
	vector<float> curvature = vector<float>(contour.size());
	//vector<int> curv_below_t_idx;
	for (int i = k; i < contour.size()-k; i++) {
		Point p0 = curvature[i - k];
		Point p1 = curvature[i];
		Point p2 = curvature[i + k];
		curvature[i] = angle_between(p0, p1, p2);
		if (curvature[i] < threshold){
			curv_below_t_idx.push_back(i);
		}
		
	}
	return curvature;
}

float FindHull::angle_between(Point p0, Point p1, Point p2) {
	Point v = p0 - p1; Point u = p2 - p1;
	return acos(v.dot(u) / (sqrt(v.dot(v)*u.dot(u))));
}


// deConstructor
FindHull::~FindHull()
{
}
