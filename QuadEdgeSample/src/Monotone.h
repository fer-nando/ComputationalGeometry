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


class VertexComp {
public:
  bool operator() (const Vertex * lhs, const Vertex * rhs) const
  {
  	if (lhs->p.y < rhs->p.y) {
  		return false;
  	} else if (rhs->p.y < lhs->p.y) {
  		return true;
  	} else {
  		if (lhs->p.x <= rhs->p.x)
  			return false;
  		else
  			return true;
  	}
  }
};

class EdgeComp {
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


typedef std::priority_queue<Vertex *,std::vector<Vertex *>,VertexComp> PriorityQueue;

class Monotone {

public:
	Monotone(Mesh &mesh, Mat &img);
	virtual ~Monotone();

	void makeMonotone(Face &f);

private:
	bool visual;
	Mat src_img;
	Mesh mesh;
	PriorityQueue queue;
	std::set<Edge *,EdgeComp> tree;
	std::map<Edge *,Vertex *,EdgeComp> helper;
	std::vector<Edge *> newEdges;
	const char* iter_window = "MakeMonotone iterativo";

	int  findVertexType(Vertex &v, Face &f);
	Edge* findLeftEdge(Vertex &v, Face &f);
	Edge* findRightEdge(Vertex &v, Face &f);
	void handleStartVertex(Vertex &v, Face &f);
	void handleEndVertex(Vertex &v, Face &f);
	void handleSplitVertex(Vertex &v, Face &f);
	void handleMergeVertex(Vertex &v, Face &f);
	void handleRegularVertex(Vertex &v, Face &f);
	void showVertex(Mat &img, Vertex &v, const Scalar &color);
	void showTree(Mat &img);
	void showNewEdges(Mat &img);
};

#endif /* MONOTONE_H_ */

