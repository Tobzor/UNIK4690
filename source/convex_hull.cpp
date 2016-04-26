#include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <iostream>
 #include <stdio.h>
 #include <stdlib.h>

 using namespace cv;
 using namespace std;
 
 Mat src; Mat src_gray;
 int thresh = 100;
 int max_thresh = 255;
 RNG rng(12345);

 /// Function header
 void thresh_callback(int, void* );

/** @function main */
 int main(int argc, char** argv)
 { 
	 // Open external camera
	 VideoCapture cap(0);
	 // check if opening succeeded
	 if (!cap.isOpened()) {
		 cout << "Camera couldn't be opened" << endl;
		 return -1;
	 };

   // Create Window
	 namedWindow("source_window", CV_WINDOW_AUTOSIZE);

	 bool while_loop = true;
	 while (while_loop) {
		 // Putting a frame from the camera into src matrix.
		 cap >> src;

		 // Convert image to gray and blur it
		 cvtColor(src, src_gray, CV_BGR2GRAY);
		 blur(src_gray, src_gray, Size(3, 3));

		 // Create trackbar
		 createTrackbar(" Threshold:", "source_window", &thresh, max_thresh, thresh_callback);

		 // Finding contour, drawing convex_hull and contours.
		 thresh_callback(0, 0);
		 
		 // Display the src matrix - in the window "source_window".
		 imshow("source_window", src);

		 // Calling quits on esc press.
		 int keycode = waitKey(30);
		 switch (keycode) {
			 case 27: // == Esc
				 while_loop = false;
				 break;
			 default: break;
		 };	
	 }
   return(0);
 }

 /** @function thresh_callback */
 void thresh_callback(int, void* )
 {
   Mat src_copy = src.clone();
   Mat threshold_output;
   vector<vector<Point> > contours;
   vector<Vec4i> hierarchy;

   /// Detect edges using Threshold
   threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );

   /// Find contours
   findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

   /// Find the convex hull object for each contour
   vector<vector<Point> >hull( contours.size() );
   for( int i = 0; i < contours.size(); i++ )
      {  convexHull( Mat(contours[i]), hull[i], false ); }

   // Draw contours + hull results
   // Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
   for( int i = 0; i< contours.size(); i++ )
      {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( src, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        drawContours( src, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
      }
 }