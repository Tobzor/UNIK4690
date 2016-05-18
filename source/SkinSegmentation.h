#pragma once
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class SkinSegmentation
{
	std::vector< cv::Rect > faces;

	int Y_min = 54; int Y_max = 200;
	int Cr_min = 140; int Cr_max = 180;
	int Cb_min = 68; int Cb_max = 136;
	float Y_range = 150;
	float Cr_range = 50;
	float Cb_range = 10;
	CascadeClassifier face_cascade;
	Mat morph_element;

public:
	SkinSegmentation();
	~SkinSegmentation();


	void find_faces(cv::Mat image, std::vector< cv::Rect >& faces, cv::CascadeClassifier face_cascade);
	void find_faces(cv::Mat image, std::vector< cv::Rect >& faces);
	void draw_faces(cv::Mat& image, std::vector< cv::Rect >faces);
	void delete_faces(Mat thresholded_image, Mat& output_image, std::vector< cv::Rect > faces, bool debug);
	void skin_segmentation(Mat frame, Mat& output_image);
};

