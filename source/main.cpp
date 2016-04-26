#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


RNG rng(12345);

cv::Mat removed_background;
cv::Mat frame;
cv::Mat gray_frame;

cv::Mat lab_frame;
Mat threshold_output;

// Function header
void thresh_callback(int, void*);

int main()
{
	cv::VideoCapture cap{ 0 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}

	cv::Mat background;
	bool background_found = false;


	namedWindow("Input");
	namedWindow("Background removed");
	namedWindow("Threshold");


	while (true)
	{
		cap >> frame;
		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
		cv::cvtColor(frame, lab_frame, cv::COLOR_LBGR2Lab);

		if (background_found) {

			background.convertTo(background, CV_32F);
			gray_frame.convertTo(gray_frame, CV_32F);
			removed_background = background - gray_frame;

			removed_background.convertTo(removed_background, CV_8U);

			// Blurring removed_background
			blur(removed_background, removed_background, Size(3, 3));

			thresh_callback(0, 0);
			
			cv::imshow("Background removed",removed_background);
		}
		std::vector<cv::Mat> spl;
		cv::split(lab_frame, spl);

		imshow("Input", frame);

		char key = cv::waitKey(30);
		if (key == ' ') {
			background = gray_frame;
			background_found = true;
		}
		else if (key >= 0) break;

	}
}

/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat src_copy = frame.clone();
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using Threshold
	threshold(removed_background, threshold_output, 0, 255, THRESH_OTSU);
	
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
	// Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(frame, contours, i, colorBlue, 1, 8, vector<Vec4i>(), 0, Point());
		drawContours(frame, hull, i, colorRed, 1, 8, vector<Vec4i>(), 0, Point());
	}
}
