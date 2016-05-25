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
bool skin_segmentation = false;
bool debug = false;
// Variables
int thresh_val;
float gun_count;
Point bullet;
Point bullet_velocity;
Point bullet_acceleration;
bool hasfired = false;

// Function headers
void toggles(char);
void drawing();
void remove_background();
// draw_circles and required variable(s)
void draw_circles(FindHull, double opacity);
double opacity = 0.9;
void draw_numbers(FindHull o, double opacity);
bool is_finger_gun(FindHull o);

// Objects
FindHull hull;
int main()
{
	cout << "Press i to print the values of the booleans" << endl;
	cv::VideoCapture cap{0};
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}
	gun_count++;

	namedWindow("Adjust segmentation");
	// Trackbar to be used when otsu is overreacting
	thresh_val = 240,
		createTrackbar("Threshold", "Adjust segmentation", &thresh_val, 255);
	char key;
	while ((key = cv::waitKey(30)) != 27) // while loop exits on "esc" press.
	{
		cap >> frame;
		toggles(key); // keyboard controls
		flip(frame, frame, 180); // flipping frame for convenience
		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
		if (skin_segmentation) {
			hull.thresh_callback(frame, 0);
			// If contours exists draw circles/numbers
			drawing();
			// Activates Debug face removal window if debug is true
			if (debug) {
				hull.debug_face = true;
			}
		}
		// "Space" pressed --> removing background
		else if (!skin_segmentation && background_found) {
			remove_background();
			hull.thresh_callback(removed_background, thresh_val);
			// If contours exists draw circles/numbers
			drawing();
			// Activate Background removed window if debug is true
			if (debug) {
				imshow("Background removed", removed_background);
			}
			if (displayContour) {
				imshow("Input", hull.drawing);
			}
		}
		// "t" key pressed == displaying contours instead of frame.
		else if (!skin_segmentation && displayContour) {
			// If background is not set - threshold with gray_frame instead.
			if (!background_found) {
				hull.thresh_callback(gray_frame, thresh_val);
				drawing();
			}
			// display contour in "Input"
			imshow("Input", hull.drawing);
		}
		// When skin_segmentation, background_found and displayContour are false
		else
		{
			imshow("Input", frame);
		}
	}
}

void drawing() 
{
	if (hull.contours.size() > 0) {
		draw_circles(hull, opacity);
		draw_numbers(hull, opacity);
		imshow("Input", frame);
	}
	else
	{
		imshow("Input", frame);
	}
}

void toggles(char key)
{
	if (key == 'i') {
		cout << boolalpha << "##############################" << endl;
		cout << "The values of the booleans are:" << endl;
		cout << "background_found: " << background_found << "\n" << "Press 'Space' to toggle." << endl;
		cout << "\n" << "displayContour: " << displayContour << "\n" << "Press 't' to toggle." << endl;
		cout << "\n" << "use_otsu: " << hull.use_otsu << "\n" << "Press 'o' to toggle." << endl;
		cout << "\n" << "skin_segmentation: " << skin_segmentation << "\n" << "Press 's' to toggle." << endl;
		cout << "\n" << "debug: " << debug << "\n" << "Press 'd' to toggle."<< endl;
		cout << "\n" << "debug curvature: " << hull.debug_curv << "\n" << "Press 'k' to toggle." << endl;
		cout << "##############################" << endl;
	}
	if (key == ' ') {
		background = gray_frame;
		background_found = !background_found;
	}
	if (key == 't') {
		displayContour = !displayContour;
	}
	if (key == 'o') {
		hull.use_otsu = !hull.use_otsu;
	}
	if (key == 's') {
		skin_segmentation = !skin_segmentation;
		hull.skin_segmentation = !hull.skin_segmentation;
	}
	if (key == 'k') {
		hull.debug_curv = !hull.debug_curv;
		if (!hull.debug_curv) {
			destroyWindow("K-curvature");
		}
	}
	if (key == 'd') {
		debug = !debug;
		hull.debug_face = !hull.debug_face;
		if (displayContour || background_found) {
			hull.debug_thresh = !hull.debug_thresh;
		}
		if (!debug) {
			destroyWindow("Input");
			destroyWindow("Background removed");
			destroyWindow("Threshold");
			destroyWindow("Debug face deletion");
		}
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
	for (int j = 0; j < o.approx_defects.size(); j++) {
		Vec4i defect = o.approx_defects[j];
		int max_distance_idx = defect[2];
		circle(overlay, o.approx_contour[max_distance_idx], radius, Scalar(255, 0, 255), thickness, lineType);
	}
	if (o.palm_radius > 0) {
		circle(overlay, o.palm_center, radius, Scalar(255, 100, 255), thickness, lineType);
		circle(overlay, o.palm_center, o.palm_radius, Scalar(255, 100, 255), 2, lineType);
	}

	if (debug) {
		if (o.bound_circle_radius > 0) {
			circle(overlay, o.bound_circle_center, o.bound_circle_radius, Scalar(255, 100, 255), 2, lineType);
		}
		for (int j = 0; j < o.semi_approx_inthull.size(); j++) {
			circle(overlay, o.semi_approx_contour[o.semi_approx_inthull[j]], radius, Scalar(255, 255, 255), thickness, lineType);
		}
	}
	for (int j = 0; j < o.fingers.size(); j++) {
		Finger currentFinger = o.fingers[j];
		Point root = currentFinger.finger_root;
		Point tip = currentFinger.getPoint();
		circle(overlay, tip, radius * 3, Scalar(255, 0, 0), thickness, lineType);
		line(overlay, root, tip, Scalar(0,0, 255), 5, 8, 0);
	}
	putText(overlay, to_string(o.direction),Point(100,50), CV_FONT_HERSHEY_COMPLEX, 1, Scalar(100, 100, 255), 2, lineType);

	if ((o.thumb_point.x + o.thumb_point.y) > 0) {
		circle(overlay, o.thumb_point, 40, Scalar(255, 255, 255), thickness, lineType);
	}
	vector<vector<Point> > contourVec;
	contourVec.push_back(o.approx_contour);
	if (debug) {
		o.draw_contour(overlay, o.semi_approx_contour,Scalar(0,0,255));
	}
	addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);

}

void draw_numbers(FindHull o, double opacity) {

	int lineType = 1;
	Mat overlay = frame.clone();
	int no_fingers = o.fingers.size();
	if (o.direction == FindHull::RIGHT) {
		for (int j = 0; j < no_fingers; j++) {
			// Right hand, draws numbers on the fingers.
			putText(overlay, to_string(no_fingers-j-1), o.fingers[j].getPoint(), CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, lineType);
		}
	}
	else {
		for (int j = 0; j < no_fingers; j++) {
			// Left/unknown hand, draws numbers on the fingers.
			putText(overlay, to_string(j), o.fingers[j].getPoint(), CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, lineType);
		}
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
				Point p1 = o.semi_approx_contour[o.fingers_idx[0]];
				Point p2 = o.semi_approx_contour[o.fingers_idx[1]];
				Point d1 = p1 - o.semi_approx_contour[o.thumb_defect[2]];
				Point d2 = p2 - o.semi_approx_contour[o.thumb_defect[2]];
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
	return;
}

bool is_finger_gun(FindHull o) {
	int num_fingers = o.fingers_idx.size();
	Vec4i thumb_defect = o.thumb_defect;
	if (num_fingers != 2) {
		return false;
	}
	int f1_idx  = o.fingers_idx[0];
	int f2_idx  = o.fingers_idx[1];
	////int midt_idx = f1_idx + 1;
	//Point p1 = o.semi_approx_contour[f1_idx];
	////Point p2 = o.approx_contour[midt_idx];
	//Point p2 = o.palm_center;
	//Point p3 = o.semi_approx_contour[f2_idx];

	Point p1 = o.semi_approx_contour[thumb_defect[0]];
	Point p2 = o.semi_approx_contour[thumb_defect[1]];
	Point p3 = o.semi_approx_contour[thumb_defect[2]];

	circle(frame, p1, 100, Scalar(0, 0, 0), -1, 8);
	circle(frame, p2, 50, Scalar(0, 0, 255), -1, 8);
	circle(frame, p3, 25, Scalar(0, 255, 0), -1, 8);

	float angle = o.angle_between(p1, p3, p2)*180/CV_PI;

	putText(frame, to_string(angle), Point(200, 200), CV_FONT_HERSHEY_COMPLEX, 4, Scalar(0, 0, 255), 4, 8);

	if ((angle > 110) || (angle < 75)) {
		return false;
	}
	return true;
}



