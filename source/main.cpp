#include "opencv2/opencv.hpp"

int main()
{
	cv::VideoCapture cap{ 1 };
	if (!cap.isOpened()) {
		throw std::runtime_error{ "Could not open VideoCapture" };
	}


	cv::Mat removed_background;
	cv::Mat frame;
	cv::Mat gray_frame;
	cv::Mat lab_frame;

	cv::Mat background;
	bool background_found = false;


	cv::namedWindow("Input");
	cv::namedWindow("Background removed");


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
			cv::imshow("Background removed",removed_background);
		}
		std::vector<cv::Mat> spl;
		cv::split(lab_frame, spl);

		cv::imshow("Input", lab_frame);

		char key = cv::waitKey(30);
		if (key == ' ') {
			background = gray_frame;
			background_found = true;
		}
		else if (key >= 0) break;

	}
}
