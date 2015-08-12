/*
 * Monotone.h
 *
 *  Created on: Aug 7, 2015
 *      Author: fernando
 */

#ifndef MONOTONE_H_
#define MONOTONE_H_

#include <set>
#include <queue>
#include <map>
#include <functional>
#include <opencv2/core/core.hpp>
#include "Edge.h"
#include "MeshGenerator.h"


class EdgeCompY {
public:
  bool operator() (const Edge* lhs, const Edge* rhs) const
  {
  	Vertex *v1 = lhs->Orig();
  	Vertex *v2 = rhs->Orig();
  	if (v1->p.y < v2->p.y) {
  		return false;
  	} else if (v2->p.y < v1->p.y) {
  		return true;
  	} else {
  		if (v1->p.x <= v2->p.x)
  			return false;
  		else
  			return true;
  	}
  }
};

class EdgeCompX {
public:
  bool operator() (const Edge* lhs, const Edge* rhs) const
  {
  	Vertex *v1 = lhs->Orig();
  	Vertex *v2 = rhs->Orig();
  	if (v1->p.x > v2->p.x) {
  		return false;
  	} else if (v2->p.x > v1->p.x) {
  		return true;
  	} else {
  		if (v1->p.y >= v2->p.y)
  			return false;
  		else
  			return true;
  	}
  }
};


typedef std::priority_queue<Edge *,std::vector<Edge *>,EdgeCompY> PriorityQueue;

class Monotone {

public:
	Monotone(Mesh *mesh, Mat &img);
	virtual ~Monotone();

	void makeMonotone(Face *f);
	void triangulate(Face *f);
	void clearNewEdges();
	void closeWindows(int delay);

private:
	bool visual;
	Mat src_img;
	Mesh *mesh;
	PriorityQueue queue;
	std::set<Edge *,EdgeCompX> tree;
	std::map<Edge *,Vertex *,EdgeCompX> helper;
	std::vector<Edge *> newEdges;
	const char* iter_window = "Triangulatizacao iterativa";

	void handleStartVertex(Edge *e);
	void handleEndVertex(Edge *e);
	void handleSplitVertex(Edge *e);
	void handleMergeVertex(Edge *e);
	void handleRegularVertex(Edge *e);

	void showVertex(Mat &img, Vertex &v, const Scalar &color);
	void showTree(Mat &img);
	void showNewEdges(Mat &img);

	Edge* insertNewEdge(Face *f, Vertex *v1, Vertex *v2);
	double getVertexAngle(Vertex *v1, Vertex *v2, Vertex *v3);
	int  findVertexType(Edge *e);
	Edge* findLeftEdge(Edge *e);
	Edge* findRightEdge(Edge *e);
};

#endif /* MONOTONE_H_ */

