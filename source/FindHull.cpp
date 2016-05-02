#include "findHull.h"

using namespace cv;
using namespace std;

// defualt constructor
findHull::findHull()
{
}


void findHull::thresh_callback(cv::Mat background_removed)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	int largest_C_area = 0;
	int largest_C_index = 0;

	/// Detect edges using Threshold
	threshold(background_removed, threshold_output, 0, 255, THRESH_OTSU);

	imshow("Threshold", threshold_output);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Find the convex hull object for each contour
	vector<vector<Point> >hull(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		convexHull(Mat(contours[i]), hull[i], false);
	}

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


	//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	Scalar colorBlue = Scalar(255, 0, 0);
	Scalar colorRed = Scalar(0, 0, 255);

	// Draw contours + hull results in this mat.
	drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	

	drawContours(drawing, contours, largest_C_index, colorBlue, 2, 8, vector<Vec4i>(), 0, Point());
	drawContours(drawing, hull, largest_C_index, colorRed, 2, 8, vector<Vec4i>(), 0, Point());

	/* This loop draws every contour.
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(drawing, contours, i, colorBlue, 2, 8, vector<Vec4i>(), 0, Point());
		drawContours(drawing, hull, i, colorRed, 2, 8, vector<Vec4i>(), 0, Point());
	}
	*/
}



// deConstructor
findHull::~findHull()
{
}
