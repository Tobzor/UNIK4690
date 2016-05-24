#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

class Finger
{

private:
	vector <Point> contour;
	Point find_finger_root();
public:
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