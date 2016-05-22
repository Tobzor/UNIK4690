#pragma once
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class SkinSegmentation
{
	std::vector< cv::Rect > faces;

	int Y_min  =  24; int Y_max  = 255;
	int Cr_min = 136; int Cr_max = 187;
	int Cb_min =  86; int Cb_max = 142;
	float Y_range = 150;
	float Cr_range = 50;
	float Cb_range = 10;
	CascadeClassifier face_cascade;
	Mat morph_element_opening, morph_element_closing;

	Mat tmp;

public:
	SkinSegmentation();
	~SkinSegmentation();

	void find_faces(cv::Mat image, std::vector< cv::Rect >& faces, cv::CascadeClassifier face_cascade);
	void find_faces(cv::Mat image, std::vector< cv::Rect >& faces);
	void draw_faces(cv::Mat& image, std::vector< cv::Rect >faces);
	void delete_faces(Mat thresholded_image, Mat& output_image, std::vector< cv::Rect > faces, bool debug);
	void skin_segmentation(Mat frame, Mat& output_image);
};

