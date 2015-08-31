/*
 * DrawFunctions.h
 *
 *  Created on: Aug 10, 2015
 *      Author: fernando
 */

#ifndef DRAWFUNCTIONS_H_
#define DRAWFUNCTIONS_H_

#include <opencv2/core/core.hpp>


const int line_width = 3;
const Scalar default_face_color = Scalar(255, 200, 200);
const Scalar highlight_face_color = Scalar(255, 0, 0);
const Scalar default_vertex_color = Scalar(0, 0, 0);
const Scalar highlight_vertex_color = Scalar(0, 0, 255);
const Scalar default_edge_color = Scalar(0, 0, 0);
const Scalar highlight_edge_color = Scalar(0, 0, 255);
const Scalar default_next_color = Scalar(0, 0, 0);
const Scalar highlight_next_color = Scalar(0, 255, 0);

void drawDashedLine(Mat &img, Point p1, Point p2, Scalar color, int dashLen, int lineType);
void drawPolygon(Mat &src, Point *pts, int npts, int width, const Scalar cFill,
		const Scalar cLine, const Scalar cPoint);
void drawMesh(Mat &src, vector<Face *>& faceList, int width,
		bool step = false, int delay = 1000);


#endif /* DRAWFUNCTIONS_H_ */
