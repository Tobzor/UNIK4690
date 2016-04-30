#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


RNG rng(12345);

// Matrices
Mat removed_background;
Mat frame;
Mat gray_frame;
Mat background;
Mat threshold_output;


vector<vector<Point> > contours;
vector<vector<Point> > contours_approx;
vector<vector<Point> >hull;
vector<vector<int> >hull_indices;
vector<vector<Vec4i> >defects;
vector<Rect> boundRects;

// Booleans
bool background_found = false;
bool use_otsu = true;


int thresh_val;

// Function header
void toggle(char);
void thresh_callback(int, void*);
void remove_background();
void draw_circles();

int main()
{
	cv::VideoCapture cap{ 0 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}

	namedWindow("Input");
	namedWindow("Background removed");
	namedWindow("Threshold");

	thresh_val = 240,

	createTrackbar("Threshold", "Threshold", &thresh_val, 255);

	char key;
	while ((key = cv::waitKey(30)) != 27)
	{
		cap >> frame;
		toggle(key);

		cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
		if (background_found) {

			remove_background();
			// Blurring removed_background
			blur(removed_background, removed_background, Size(5, 5));

			thresh_callback(0, 0);

			draw_circles();
			cv::imshow("Background removed",removed_background);
		}
		imshow("Input", frame);

	}
}

void toggle(char key)
{
	if (key == ' ') {
		background = gray_frame;
		background_found = true;
	}
	else if (key == 'o') use_otsu = !use_otsu;
}


/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat src_copy = frame.clone();
	vector<Vec4i> hierarchy;

	int thresh_type = THRESH_BINARY;
	if (use_otsu) thresh_type |= THRESH_OTSU;
	/// Detect edges using Threshold
	threshold(removed_background, threshold_output, thresh_val, 255, thresh_type);

	/// Morphology - first opening, then closing
	erode(threshold_output, threshold_output, cv::Mat());
	dilate(threshold_output, threshold_output, cv::Mat());


	dilate(threshold_output, threshold_output, cv::Mat());
	erode(threshold_output, threshold_output, cv::Mat());
	
	imshow("Threshold", threshold_output);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));



	/// Find the convex hull object for each contour
	hull         = vector<vector<Point> >(contours.size());
	defects      = vector<vector<Vec4i> >(contours.size());
	hull_indices = vector<vector<int> >(contours.size());
	boundRects   = vector<Rect>(contours.size());

	contours_approx.resize(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_approx[i], 40, true);
		boundRects[i] = boundingRect(Mat(contours_approx[i]));
	}

	for (int i = 0; i < contours.size(); i++)
	{

		convexHull(Mat(contours_approx[i]), hull[i], false);
		convexHull(Mat(contours_approx[i]), hull_indices[i], false);
		convexityDefects(contours_approx[i],hull_indices[i], defects[i]);
	}

	//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	Scalar colorBlue = Scalar(255, 0, 0);
	Scalar colorRed = Scalar(0, 0, 255);

	// Draw contours + hull results
	// Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(frame, contours_approx, i, colorBlue, 1, 8, vector<Vec4i>(), 0, Point());
		drawContours(frame, hull, i, colorRed, 1, 8, vector<Vec4i>(), 0, Point());
	}
}

void remove_background() 
{

	background.convertTo(background, CV_32F);
	gray_frame.convertTo(gray_frame, CV_32F);
	removed_background = gray_frame - background;

	removed_background.convertTo(removed_background, CV_8U);
}

void draw_circles() {

	int thickness = -1;
	int lineType = 8;
	int m = frame.rows;
	int n = frame.cols;
	float radius = 5;


	Point center = Point(0.5f*m, 0.5f*n);
	cv::Mat overlay = frame.clone();
		
	//circle(overlay, center, radius, Scalar(255, 255, 255, 0.5), thickness, lineType);

	double opacity = 0.9;

	
	//circle(frame,center,radius,Scalar(255, 255, 255,0.5),thickness,lineType);

	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < hull[i].size(); j++) {
			circle(overlay, hull[i][j], radius, Scalar(255, 255, 255), thickness, lineType);
		}

		for (int j = 0; j < defects[i].size(); j++) {
			Vec4i defect = defects[i][j];
			int max_distance_idx = defect[2];

			circle(overlay, contours_approx[i][max_distance_idx], radius, Scalar(255, 0, 255), thickness, lineType);
		}

		rectangle(frame, boundRects[i].tl(), boundRects[i].br(), Scalar(255, 0, 0), 2, 8, 0);
	}


	cv::addWeighted(overlay, opacity, frame, 1.0 - opacity, 0.0, frame);
}