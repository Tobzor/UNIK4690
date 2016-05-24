#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class Finger
{

private:
	Point find_finger_root();
public:
	vector <Point> contour;
	int idx;
	bool isThumb;
	Point getPoint();
	Vec4i defectRight;
	Vec4i defectLeft;
	Finger(int i, vector<Point> c, Vec4i dl, Vec4i dr);
	~Finger();
	Finger();
	Point finger_root;


};