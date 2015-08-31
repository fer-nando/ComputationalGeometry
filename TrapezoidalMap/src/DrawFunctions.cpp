/*
 * DrawFunctions.cpp
 *
 *  Created on: Aug 10, 2015
 *      Author: fernando
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <iostream>
#include <stdio.h>
#include "Edge.h"
#include "EdgeFunctions.h"
#include "DrawFunctions.h"

using namespace std;
using namespace cv;

extern const char * main_window;


void drawDashedLine(Mat &img, Point p1, Point p2, Scalar color, int dashLen, int lineType) {
  bool dash = true;
  Point temp1, temp2;
	LineIterator it(img, p1, p2, 8);

	temp1 = it.pos();

  for(int i = 1; i < it.count; i++, it++) {
		if ( i%dashLen == 0 ) {
			if (dash) {
				temp2 = it.pos();
				line(img, temp1, temp2, color, 3, lineType);
				dash = false;
			} else {
				temp1 = it.pos();
				dash = true;
			}
		}
	}
}

/**
 *
 */
void drawPolygon(Mat &src, Point *pts, int npts, int width, const Scalar cFill,
		const Scalar cLine, const Scalar cPoint) {
	int i;

	/// Fill the polygon
	const Point * ppts[] = { pts };
	const int pnpts[] = { npts };
	fillPoly(src, ppts, pnpts, 1, cFill, CV_AA);

	/// Draw lines
	for (i = 0; i < npts - 1; i++) {
		line(src, pts[i], pts[i + 1], cLine, width, CV_AA);
	}
	line(src, pts[0], pts[npts - 1], cLine, width, CV_AA);

	/// Draw points
	const int radius = 2 * width;
	for (i = 0; i < npts; i++) {
		circle(src, pts[i], radius, cPoint, -1, CV_AA);
	}
}

void drawMesh(Mat &src, vector<Face *>& faceList, int width, bool step, int delay) {
	int i, j;
	vector<Vertex> vertices;
	Point *pts;

	//cout << "facelist = " << faceList.size() << endl;
	for (i = 0; i < (int) faceList.size(); i++) {
		int faceOrder;

		Edge *e = faceList[i]->getEdge();
		faceOrder = getFaceOrder(e, vertices);
		pts = getPointsFromVertexList(vertices);

		//cout << "face " << i << ": " << faceList[i] << "; order=" << faceOrder << endl;

		/// Draw the first point
	const int radius = 2 * width;
		circle(src, pts[0], radius, default_vertex_color, -1);
		for (j = 0; j < faceOrder - 1; j++) {
			/// Draw the current line
			line(src, pts[j], pts[j + 1], default_edge_color, width, CV_AA);

			if (step) {
				/// Show the image
				imshow(main_window, src);
				waitKey(delay);
			}
			/// Draw the next point
			circle(src, pts[j + 1], radius, default_vertex_color, -1, CV_AA);
		}

		/// Draw the current polygon
		drawPolygon(src, pts, faceOrder, width, default_face_color, default_edge_color,
				default_vertex_color);

		if (step) {
			/// Show the image
			imshow(main_window, src);
		}
	}
	//imshow(main_window, src);
}
