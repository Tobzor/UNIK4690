#include "findHull.h"

using namespace cv;
using namespace std;

// defualt constructor
findHull::findHull ()
{
}


void findHull::thresh_callback(cv::Mat background_removed)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

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

	//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	Scalar colorBlue = Scalar(255, 0, 0);
	Scalar colorRed = Scalar(0, 0, 255);

	// Draw contours + hull results
	drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(drawing, contours, i, colorBlue, 1, 8, vector<Vec4i>(), 0, Point());
		drawContours(drawing, hull, i, colorRed, 1, 8, vector<Vec4i>(), 0, Point());
	}
}



// deConstructor
findHull::~findHull()
{
}
