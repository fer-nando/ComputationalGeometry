#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <cstdio>
#include <cstdlib>
#include <locale>

 using namespace cv;
 using namespace std;

 Mat src; Mat src_gray; Mat src_thld;
 Mat src2; Mat src2_gray; Mat src2_thld;
 int thresh = 100;
 int max_thresh = 255;
 RNG rng(12345);

 /// Function header
 void thresh_callback(int, void* );
 void ConvexHull(Mat &img, char* window, char* filename);

/** @function main */
int main( int argc, char** argv )
 {
   /// Load source image and convert it to gray
	src = imread( "perc3.png", 1 );
	src2 = imread( "perc4.png", 1 );

   /// Convert image to gray and blur it
   cvtColor( src, src_gray, CV_BGR2GRAY );
   cvtColor( src2, src2_gray, CV_BGR2GRAY );
   //blur( src_gray, src_gray, Size(5,5) );

   /// Detect edges using Threshold
   threshold( src_gray, src_thld, thresh, 255, THRESH_BINARY_INV );
   threshold( src2_gray, src2_thld, thresh, 255, THRESH_BINARY_INV );

   /// Create Window
   char* source1_window = "Source1";
   char* source2_window = "Source2";
   namedWindow( source1_window, WINDOW_AUTOSIZE );
   namedWindow( source2_window, WINDOW_AUTOSIZE );
   imshow( source1_window, src_thld );
   imshow( source2_window, src2_thld );

   createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
   thresh_callback( 0, 0 );

   waitKey(0);
   return(0);
 }

 /** @function thresh_callback */
 void thresh_callback(int, void* )
 {

	 Mat img_not(src_thld.size(), src_thld.type());
	 Mat img_diff(src_thld.size(), src_thld.type());
	 bitwise_not(src_thld, img_not);
	 bitwise_and( img_not, src2_thld, img_diff);

   char* diff_window = "Diff";
   namedWindow( diff_window, WINDOW_AUTOSIZE );
   imshow( diff_window, img_diff );

	 ConvexHull(src_thld, "ConvexHull 1", "mesh1.obj");
	 ConvexHull(img_diff, "ConvexHull diff", "mesh2.obj");

 }

 void ConvexHull(Mat &img, char* window, char* filename) {

	 std::setlocale(LC_ALL, "C");
	 FILE *f = fopen(filename, "w");

   vector<vector<Point> > contours;
   vector<Vec4i> hierarchy;
   vector<vector<int> > faces;
   int v = 0;

   /// Find contours
   findContours( img, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

   /// Find the convex hull object for each contour
   vector<vector<Point> >hull( contours.size() );
   for( int i = 0; i < contours.size(); i++ )
      {  convexHull( Mat(contours[i]), hull[i], true ); }

   /// Draw contours + hull results
   Mat drawing = Mat::zeros( img.size(), CV_8UC3 );
   for( int i = 0; i< contours.size(); i++ )
      {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, Scalar(255,255,255), 1, 8, vector<Vec4i>(), 0, Point() );
        drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );

        vector<Point> contour = hull.at(i);
        vector<int> face(contour.size());
        for( int j = 0; j < contour.size(); j++ ) {
        	Point p = contour.at(j);
        	float x = (double) p.x / 433;
        	float y = (double) p.y / 342;
        	float z = 0;
        	fprintf(f, "v %1.3f %1.3f %1.3f\n", x, y, z);
        	printf("v %1.3f %1.3f %1.3f\n", x, y, z);

        	++v;
        	face[j] = v;

        	circle( drawing, contour.at(j), 3, color, -1, CV_AA);

        }
        faces.push_back(face);

      }

   fprintf(f, "\n");
   printf("\n");

   for ( int i = 0; i < faces.size(); i++ ) {
  	 vector<int> face = faces.at(i);
  	 fprintf(f, "f");
  	 printf("f");
  	 for( int j = 0; j < face.size(); j++ ) {
  		 fprintf(f, " %i", face.at(j));
  		 printf(" %i", face.at(j));
  	 }
  	 fprintf(f, "\n");
  	 printf("\n");
   }

   fclose(f);

   /// Show in a window
   namedWindow( window, WINDOW_AUTOSIZE );
   imshow( window, drawing );
 }
