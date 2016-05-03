#include "opencv2/opencv.hpp"

#include "FindHull.h"

using namespace cv;
using namespace std;

// Matrices
Mat removed_background;
Mat frame;;
Mat gray_frame;
Mat blurred_frame;
Mat background;
Mat threshold_output;

vector<vector<Point> > contours;
vector<vector<Point> > contours_approx;
vector<vector<Point> >hull;
vector<vector<int> >hull_indices;
vector<vector<Vec4i> >defects;
vector<Rect> boundRects;

vector<Point> max_circle_centers;
vector<float> max_circle_radius;

// Booleans
bool background_found = false;
bool displayContour = false;
bool displayBackground = true;
bool use_otsu = true;

int thresh_val;

// Function header
void toggles(char);
void remove_background();
void draw_circles(vector<Point>  contours_approx, vector<Point> hull, vector<Vec4i> defects, Rect boundRect, Point max_circle_center, float max_circle_radius, Point2f bound_circle_center, float bound_circle_radius, double opacity);

int main()
{
	cv::VideoCapture cap{ 0 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}

	namedWindow("Input");
	namedWindow("Background removed");
	namedWindow("Threshold");
	namedWindow("Circles");

	// Finnes sikkert bedre måte å gjøre det på, men IT WORKS atleast.
	FindHull hull;

	thresh_val = 240,

		createTrackbar("Threshold", "Threshold", &thresh_val, 255);

	char key;
	while ((key = cv::waitKey(30)) != 27)
	{
		cap >> frame;
		toggles(key);

		flip(frame, frame, 180);

		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

		// "Space" key pressed == removing background, blurring it, and using this in thresholding.
		if (background_found) {
			remove_background();
			hull.thresh_callback(blurred_frame);
			imshow("Background removed", removed_background);
		}
		// "t" key pressed == displaying contours instead of frame.
		if (displayContour) {

			// If background is not set - threshold with gray_frame instead.
			if (!background_found) {
				// Blurring gray_frame
				blur(gray_frame, blurred_frame, Size(3, 3));
				hull.thresh_callback(blurred_frame);
			}
			// display contour in "Input"
			imshow("Input", hull.drawing);
		}
		else {
			imshow("Input", frame);
		}

		if (hull.contours.size() > 0) {
			draw_circles(hull.contour_approx, hull.approx_hull, hull.approx_defects, hull.boundRect, hull.circle_center, hull.circle_radius, hull.bound_circle_center, hull.bound_circle_radius, 0.9);
			imshow("Circles", frame);
		}
	}
}


void toggles(char key)
{
	if (key == ' ') {
		background = gray_frame;
		background_found = !background_found;
	}
	if (key == 't') {
		displayContour = !displayContour;
	}
	if (key == 'o') {
		use_otsu = !use_otsu;
	}
}

// Removes the background.
void remove_background()
{
	background.convertTo(background, CV_32F);
	gray_frame.convertTo(gray_frame, CV_32F);
	removed_background = gray_frame - background;
	removed_background.convertTo(removed_background, CV_8U);
	// Blurring removed_background
	blur(removed_background, blurred_frame, Size(3, 3));
}

void draw_circles(vector<Point>  contours_approx, vector<Point> hull, vector<Vec4i> defects, Rect boundRect, Point max_circle_center, float max_circle_radius, Point2f bound_circle_center, float bound_circle_radius,  double opacity) {

	int thickness = -1;
	int lineType = 8;
	int m = frame.rows;
	int n = frame.cols;
	float radius = 5;


	Point center = Point(0.5f*m, 0.5f*n);
	Mat overlay = frame.clone();

	//circle(overlay, center, radius, Scalar(255, 255, 255, 0.5), thickness, lineType);



	//circle(frame,center,radius,Scalar(255, 255, 255,0.5),thickness,lineType);

	for (int j = 0; j < hull.size(); j++) {
		circle(overlay, hull[j], radius, Scalar(255, 255, 255), thickness, lineType);
		line(overlay, max_circle_center, hull[j], Scalar(0, 255, 255), 5, 8, 0);
	}

	for (int j = 0; j < defects.size(); j++) {
		Vec4i defect = defects[j];
		int max_distance_idx = defect[2];

		circle(overlay, contours_approx[max_distance_idx], radius, Scalar(255, 0, 255), thickness, lineType);
	}

	rectangle(overlay, boundRect.tl(), boundRect.br(), Scalar(255, 0, 0), 2, 8, 0);
	circle(overlay, Point(boundRect.x, boundRect.y), radius, Scalar(255, 100, 255), thickness, lineType);
	if (max_circle_radius > 0) {
		circle(overlay, max_circle_center, radius, Scalar(255, 100, 255), thickness, lineType);
		circle(overlay, max_circle_center, max_circle_radius, Scalar(255, 100, 255), 2, lineType);
	}
	if (bound_circle_radius > 0) {
		circle(overlay, bound_circle_center, bound_circle_radius, Scalar(255, 100, 255), 2, lineType);
	}

	addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);
}