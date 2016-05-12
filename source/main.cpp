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
float gun_count;
Point bullet;
Point bullet_velocity;
Point bullet_acceleration;
bool hasfired = false;

// Function headers
void toggles(char);
void remove_background();

// draw_circles and required variable(s)
void draw_circles(FindHull, double opacity);
double opacity = 0.9;
void draw_numbers(FindHull o, double opacity);
bool is_finger_gun(FindHull o);
int main()
{
	cv::VideoCapture cap{ 0};
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}
	FindHull hull;
	gun_count++;

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
			draw_numbers(hull, opacity);
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
	for (int i = 0; i < o.fingers_idx.size(); i++) {

		line(overlay, o.circle_center, o.approx_contour[o.fingers_idx[i]], Scalar(0, 255, 255), 5, 8, 0);
		circle(overlay, o.approx_contour[o.fingers_idx[i]], radius * 3, Scalar(255, 0, 0), thickness, lineType);
	}
	//int thumb_index = o.find_thumb();
	//circle(overlay, o.approx_contour[thumb_index], radius*3, Scalar(255, 255, 255), thickness, lineType);
	vector<vector<Point> > contourVec;
	contourVec.push_back(o.approx_contour);
	drawContours(overlay, contourVec, 0, Scalar(0, 255, 255), 2, 8, vector<Vec4i>(), 0, Point());

	addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);

}

void draw_numbers(FindHull o, double opacity) {

	int lineType = 1;
	Mat overlay = frame.clone();
	for (int j = 0; j < o.fingers_idx.size(); j++) {
		putText(overlay, to_string(j+1), o.approx_contour[o.fingers_idx[j]], CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, lineType);
		
	}
	putText(overlay, to_string(o.fingers_idx.size()), Point(50,50), CV_FONT_HERSHEY_COMPLEX, 2, Scalar(100, 100, 255), 1, lineType);
	//putText(overlay, to_string(gun_count), Point(100, 100), CV_FONT_HERSHEY_COMPLEX, 2, Scalar(100, 100, 255), 1, lineType);
	if (is_finger_gun(o)) {
		if (gun_count > 25) {
			hasfired = false;
			gun_count = 0;
		}
		if (gun_count > 10) {
			
			putText(overlay, "FIRE!", Point(200, 200), CV_FONT_HERSHEY_COMPLEX, 4, Scalar(0, 0, 255), 4, lineType);
			if (hasfired) {
				bullet_velocity = bullet_velocity + bullet_acceleration;
				bullet = bullet + bullet_velocity;
			}
			else {
				hasfired = true;
				Point p1 = o.approx_contour[o.fingers_idx[0]];
				Point p2 = o.approx_contour[o.fingers_idx[1]];
				Point d1 = p1 - o.circle_center;
				Point d2 = p2 - o.circle_center;
				Point d;
				Point p;
				if (sqrt(d1.dot(d1)) > sqrt(d2.dot(d2))) {
					d = d1;
					p = p1;
				}
				else {
					d = d2;
					p = p2;
				}
				bullet_velocity = 50*d/sqrt(d.dot(d));
				bullet_acceleration = bullet_velocity / 60;
				bullet = p;
			}
			circle(overlay, bullet, 10, Scalar(0, 255, 0), -1, lineType);
		}
		else if (gun_count > 5) {

			putText(overlay, to_string(int(10-gun_count)), Point(200, 200), CV_FONT_HERSHEY_COMPLEX, 4, Scalar(0, 0, 255), 4, lineType);
		}
		else {

			putText(overlay, "AIM", Point(200, 200), CV_FONT_HERSHEY_COMPLEX, 4, Scalar(0, 0, 255), 4, lineType);
		}

		gun_count = gun_count + 1.0f;
	}
	else {
		gun_count = 0;
		hasfired  = false;
	}
	addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);
}

bool is_finger_gun(FindHull o) {
	int num_fingers = o.fingers_idx.size();
	if (num_fingers != 2) {
		return false;
	}
	int f1_idx  = o.fingers_idx[0];
	int f2_idx  = o.fingers_idx[1];
	//int midt_idx = f1_idx + 1;
	Point p1 = o.approx_contour[f1_idx];
	//Point p2 = o.approx_contour[midt_idx];
	Point p2 = o.circle_center;
	Point p3 = o.approx_contour[f2_idx];
	float angle = o.angle_between(p1, p2, p3)*180/CV_PI;

	if ((angle > 100) || (angle < 75)) {
		return false;
	}
	return true;
}



