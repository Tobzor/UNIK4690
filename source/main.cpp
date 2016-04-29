#include "opencv2/opencv.hpp"

#include "findHull.h"

using namespace cv;
using namespace std;


RNG rng(12345);

// Matrices
Mat removed_background;
Mat frame;
Mat gray_frame;
Mat blurred_frame;
Mat background;
Mat threshold_output;

// Booleans
bool background_found = false;
bool displayContour = false;
bool displayBackground = true;
// Function header
void toggles(char);
void remove_background();

int main()
{
	cv::VideoCapture cap{ 0 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}

	namedWindow("Input");
	namedWindow("Background removed");
	namedWindow("Threshold");

	// Finnes sikkert bedre måte å gjøre det på, men IT WORKS atleast.
	findHull hull;

	char key;
	while ((key = cv::waitKey(30)) != 27)
	{
		cap >> frame;
		toggles(key);

		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

		// "Space" key pressed == removing background, and using this in thresholding.
		if (background_found) {
			remove_background();
			cv::imshow("Background removed", removed_background);
			hull.thresh_callback(removed_background);
		}
		// "t" key pressed == displaying contours instead of frame.
		if (displayContour) {

			// If background is not set - threshold with gray_frame instead.
			if (!background_found) {
				// Blurring gray_frame
				blur(gray_frame, blurred_frame, Size(3, 3));
				hull.thresh_callback(gray_frame);
			}
			// display contour in "Input"
			imshow("Input", hull.drawing);
		}
		else {
			imshow("Input", frame);
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
}

// Removes the background.
void remove_background()
{
	background.convertTo(background, CV_32F);
	gray_frame.convertTo(gray_frame, CV_32F);
	removed_background = background - gray_frame;
	removed_background.convertTo(removed_background, CV_8U);

	// Blurring removed_background
	blur(removed_background, blurred_frame, Size(3, 3));
}