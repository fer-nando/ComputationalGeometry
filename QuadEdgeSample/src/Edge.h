/*
 * Edge.h
 *
 *  Created on: Jul 8, 2015
 *      Author: fernando
 */

#ifndef EDGE_H_
#define EDGE_H_


#include "opencv2/core/core.hpp"

class QuadEdge;
class Vertex;
class Face;

using namespace cv;


/**
 * Edge class
 */
class Edge {

public:

	static Edge *makeEdge();
	static Edge *makeEdge(Vertex *vs, Vertex *ve, Face *fl, Face *fr);
	static void killEdge(Edge *e);
	static void splice(Edge *a, Edge *b);

	Edge();
	~Edge();

	void setOrig(Vertex *orig);
	Vertex *Orig();
	void setDest(Vertex *dest);
	Vertex *Dest();
	void setLeft(Face *left);
	Face *Left();
	void setRight(Face *right);
	Face *Right();

	Edge *Rot();
	Edge *InvRot();
	Edge *Sym();
	Edge *Onext();
	Edge *Dnext();
	Edge *Lnext();
	Edge *Rnext();
	Edge *Oprev();
	Edge *Dprev();
	Edge *Lprev();
	Edge *Rprev();

private:

	unsigned int r;
	Edge *next;
	Vertex *orig;
	Face *left;

	friend QuadEdge;

};

/**
 * Vertex class
 */
class Vertex {

public:
	cv::Point p;

	Vertex() {
		p = cv::Point(0,0);
		edge = NULL;
	}
	Vertex(int x, int y) {
		this->p = cv::Point(x,y);
		edge = NULL;
	}
	~Vertex() {}

	void setEdge(Edge *e) { edge = e; }
	Edge *getEdge() { return edge; }

private:
	Edge *edge;

};

/**
 * Face class
 */
class Face {

public:

	Face() {
		edge = NULL;
	}
	Face(Edge* e) {
		this->edge = e;
	}
	~Face() {}

	void setEdge(Edge *e) { edge = e; }
	Edge *getEdge() { return edge; }

private:
	Edge *edge;

};

#endif /* EDGE_H_ */
