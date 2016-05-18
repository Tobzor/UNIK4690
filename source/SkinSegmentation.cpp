#include "SkinSegmentation.h"



void SkinSegmentation::skin_segmentation(Mat frame, Mat& output_image) {

	cv::cvtColor(frame, frame, cv::COLOR_BGR2YCrCb);

	createTrackbar("Y min", "Adjust segmentation", &Y_min, 255);
	createTrackbar("Y max", "Adjust segmentation", &Y_max, 255);
	createTrackbar("Cr min", "Adjust segmentation", &Cr_min, 255);
	createTrackbar("Cr max", "Adjust segmentation", &Cr_max, 255);
	createTrackbar("Cb min", "Adjust segmentation", &Cb_min, 255);
	createTrackbar("Cb max", "Adjust segmentation", &Cb_max, 255);

	cv::Scalar skin_min = cv::Scalar(Y_min, Cr_min, Cb_min);
	cv::Scalar skin_max = cv::Scalar(Y_max, Cr_max, Cb_max);

	cv::GaussianBlur(frame, frame, cv::Size(0, 0), 2);

	cv::inRange(frame, skin_min, skin_max, output_image);

	cv::morphologyEx(output_image, output_image, cv::MORPH_CLOSE, morph_element);
	cv::morphologyEx(output_image, output_image, cv::MORPH_OPEN, morph_element);

	cv::cvtColor(frame, frame, cv::COLOR_YCrCb2BGR);
}

void SkinSegmentation::delete_faces(Mat thresholded_image, Mat& output_image, std::vector< cv::Rect > faces, bool debug = false) {
	cv::Mat output_image_color;
	cv::cvtColor(output_image, output_image_color, CV_GRAY2BGR);

	for (int i = 0; i < faces.size(); i++) {
		cv::Rect r = faces[i];
		cv::Point center = cv::Point(r.x + r.width*0.5, r.y + r.height*0.5);
		cv::circle(output_image_color, center, 5, cv::Scalar(0, 255, 0), -1, 8);

		int search_size = 40;
		for (int k = -search_size*0.5; k < search_size*0.5; k++) {
			for (int l = -search_size*0.5; l < search_size*0.5; l++) {
				cv::Point p = center + Point(k, l);
				uchar val = output_image.at<uchar>(p);
				if (val > 0) {
					if (debug)cv::floodFill(output_image_color, p, cv::Scalar(255.0, 200.0, 0.0), 0);
					cv::floodFill(output_image, p, 0, 0);
				}
			}
		}
	}
	if (debug) {
		draw_faces(output_image_color, faces);
		cv::imshow("Debug face deletion", output_image_color);
	}
}
void SkinSegmentation::find_faces(cv::Mat image, std::vector< cv::Rect >& faces, cv::CascadeClassifier face_cascade) {
	cv::Size image_size = image.size();
	cv::Size max_size = cv::Size(0.9*image_size.width, 0.9*image_size.height);
	cv::Size min_size = cv::Size(0.1*image_size.width, 0.1*image_size.height);
	face_cascade.detectMultiScale(image, faces, 1.05, 6, 0, min_size);
}
void SkinSegmentation::find_faces(cv::Mat image, std::vector< cv::Rect >& faces) {
	find_faces(image, faces, face_cascade);
}
void SkinSegmentation::draw_faces(cv::Mat& image, std::vector< cv::Rect >faces) {
	for (int i = 0; i < faces.size(); i++) {
		cv::Rect r = faces[i];
		cv::rectangle(image, r.tl(), r.br(), cv::Scalar(255, 0, 0), 2, 8, 0);
	}
}

SkinSegmentation::SkinSegmentation()
{
	if (!face_cascade.load("C:/unik4690/opencv_build/unik4690_v2016_v1/etc/haarcascades/haarcascade_frontalface_default.xml")) {
		throw std::runtime_error{ "Could not load cascade classifier" };
	}

	morph_element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
}


SkinSegmentation::~SkinSegmentation()
{
}
