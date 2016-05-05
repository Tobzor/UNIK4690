#include "opencv2/opencv.hpp"

#include "FindHull.h"

using namespace cv;
using namespace std;

// Matrices
Mat frame; // starter frame
Mat gray_frame; // converted frame, used as default thresholding frame
Mat removed_background; // storing frame when background is removed
Mat background; // used in remove background

// Booleans
bool background_found = false;
bool displayContour = false;
bool displayBackground = true;
bool use_otsu = true;

// Variables
int thresh_val;

// Function headers
void toggles(char);
void remove_background();

// draw_circles and required variable(s)
void draw_circles(FindHull, double opacity);
double opacity = 0.9;
int main()
{
	cv::VideoCapture cap{ 0 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}
	FindHull hull;

	namedWindow("Input"); // either contours + hull or unedited frame
	namedWindow("Background removed"); // active when background is removed
	namedWindow("Threshold"); // active when thresholding (any frame)
	namedWindow("Circles"); // Used for displaying circles and stuff

	// Trackbar to be used when otsu is overreacting
	thresh_val = 240,

		createTrackbar("Threshold", "Threshold", &thresh_val, 255);


	char key;
	while ((key = cv::waitKey(30)) != 27) // while loop exits on "esc" press.
	{
		cap >> frame;
		toggles(key); // keyboard controls
		flip(frame, frame, 180); // flipping frame for convenience
		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

		// "Space" pressed --> removing background
		if (background_found) {
			remove_background();
			hull.thresh_callback(removed_background, thresh_val, use_otsu);
			imshow("Background removed", removed_background);
		}
		// "t" key pressed == displaying contours instead of frame.
		if (displayContour) {
			// If background is not set - threshold with gray_frame instead.

			if (!background_found) {
				hull.thresh_callback(gray_frame, thresh_val, use_otsu);
			}
			// display contour in "Input"
			imshow("Input", hull.drawing);
		}
		else {
			imshow("Input", frame);
		}

		if (hull.contours.size() > 0) {
			draw_circles(hull, opacity);
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
}

void draw_circles(FindHull o, double opacity)
{
	int thickness = -1;
	int lineType = 8;
	int m = frame.rows;
	int n = frame.cols;
	float radius = 5;
	Point center = Point(0.5f*m, 0.5f*n);
	Mat overlay = frame.clone();

	//circle(overlay, center, radius, Scalar(255, 255, 255, 0.5), thickness, lineType);
	//circle(frame,center,radius,Scalar(255, 255, 255,0.5),thickness,lineType);
	for (int j = 0; j < o.approx_hull.size(); j++) {
		circle(overlay, o.approx_hull[j], radius, Scalar(255, 255, 255), thickness, lineType);
		line(overlay, o.circle_center, o.approx_hull[j], Scalar(0, 255, 255), 5, 8, 0);
	}
	for (int j = 0; j < o.approx_defects.size(); j++) {
		Vec4i defect = o.approx_defects[j];
		int max_distance_idx = defect[2];
		circle(overlay, o.approx_contour[max_distance_idx], radius, Scalar(255, 0, 255), thickness, lineType);
	}
	rectangle(overlay, o.boundRect.tl(), o.boundRect.br(), Scalar(255, 0, 0), 2, 8, 0);
	circle(overlay, Point(o.boundRect.x, o.boundRect.y), radius, Scalar(255, 100, 255), thickness, lineType);
	if (o.circle_radius > 0) {
		circle(overlay, o.circle_center, radius, Scalar(255, 100, 255), thickness, lineType);
		circle(overlay, o.circle_center, o.circle_radius, Scalar(255, 100, 255), 2, lineType);
	}
	if (o.bound_circle_radius > 0) {
		circle(overlay, o.bound_circle_center, o.bound_circle_radius, Scalar(255, 100, 255), 2, lineType);
	}
	for (int i = 0; i < o.curv_below_t_idx.size(); i++) {
		int idx = o.curv_below_t_idx[i];
		//cout << idx << ", size: " << o.approx_contour.size()<< "\n";
		Point p = o.approx_contour[idx];
		circle(overlay, p, 2, Scalar(0, 0, 255), thickness, lineType);
		int test2 = 0;
	}
	addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);

}



