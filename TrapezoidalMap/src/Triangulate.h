/*
 * Triangulate.h
 *
 *  Created on: Aug 7, 2015
 *      Author: fernando
 */

#ifndef TRIANGULATE_H_
#define TRIANGULATE_H_

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

class Triangulate {

public:
	Triangulate(Mesh *mesh, Mat &img, const char *win);
	virtual ~Triangulate();

	void makeMonotone(Face *f);
	void triangulate(Face *f);
	void clearNewEdges();
	void closeWindows(int delay);

private:
	bool visual;
	Mat src_img, img;
	Mesh *mesh;
	PriorityQueue queue;
	std::vector<Edge *> chain;
	std::set<Edge *,EdgeCompX> tree;
	std::map<Edge *,Vertex *,EdgeCompX> helper;
	std::vector<Edge *> newEdges;
	const char* iter_window;

	void handleStartVertex(Edge *e);
	void handleEndVertex(Edge *e);
	void handleSplitVertex(Edge *e);
	void handleMergeVertex(Edge *e);
	void handleRegularVertex(Edge *e);
	void handleSameChain(Edge *&e, Edge *&lastE, int currChain);
	void handleOppositeChain(Edge *&e, Edge *&lastE, int currChain);

	void showVertex(Mat &img, Vertex &v, const Scalar &color);
	void showTree(Mat &img);
	void showNewEdges(Mat &img);
	void showReflexChain(Mat &img, int dir);

	Edge* insertNewEdge(Face *f, Vertex *v1, Vertex *v2);
	double getVertexAngle(Vertex *v1, Vertex *v2, Vertex *v3);
	int  findVertexType(Edge *e);
	Edge* findLeftEdge(Edge *e);
};

#endif /* TRIANGULATE_H_ */

