#include "Finger.h"
Finger::Finger(int i, vector<Point> c, Vec4i dl, Vec4i dr )
{
	idx = i; contour = c; defectLeft = dl; defectRight = dr;
	Point d1 = contour[dl[2]]; // left gap idx
	Point d2 = contour[dr[2]]; // right gap idx

	finger_root = find_finger_root();

}
Point Finger::find_finger_root() {
	int k = 4;
	int p0_idx = idx - k;
	int p2_idx = idx + k;

	int N = contour.size();
	if (k > contour.size()) return Point(-1,-1);
	if (p0_idx < 0) {
		p0_idx = p0_idx + N;
	}
	if (p2_idx > N - 1) {
		p2_idx = p2_idx - N;
	}

	Point p0 = contour[p0_idx];
	Point p1 = contour[idx];
	Point p2 = contour[p2_idx];


	Point cross_root = p2 - p0;
	return p0 + 0.5f*cross_root;
}
Finger::Finger() {}
Point Finger::getPoint() {
	return contour[idx];
}
Finger::~Finger()
{

}