/*
 * EdgeFunctions.h
 *
 *  Created on: Jul 8, 2015
 *      Author: fernando
 */

#ifndef EDGEFUNCTIONS_H_
#define EDGEFUNCTIONS_H_

#include "opencv2/core/core.hpp"
#include "Edge.h"

int getVertexOrder(Edge *e);
int getVertexOrder(Edge *e, cv::vector<cv::Point>& pts);
int getFaceOrder(Edge *e);
int getFaceOrder(Edge *e, cv::vector<Vertex>& pts);
Point *getPointsFromVertexList(vector<Vertex>& vertexList);
Edge *getVerticesEdge(Vertex *v1, Vertex *v2);
Point getFaceCentroid(Face *f);
int leftOn(Point a, Point b, Point c);
bool intersectEdges(Edge *a, Edge *b, Point *pi);

Edge * splitFace(Face *f, Vertex *v1, Vertex *v2);

#endif /* EDGEFUNCTIONS_H_ */
