#include "Finger.h"
Finger::Finger(int i, vector<Point> c, Vec4i dl, Vec4i dr )
{
	idx = i; contour = c; defectLeft = dl; defectRight = dr;
}
Point Finger::getPoint() {
	return contour[idx];
}
Finger::~Finger()
{

}