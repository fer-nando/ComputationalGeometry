/*
 * EdgeFunctions.cpp
 *
 *  Created on: Jul 8, 2015
 *      Author: fernando
 */

#include "EdgeFunctions.h"
#include <iostream>

using namespace cv;
using namespace std;


int leftOn(Point a, Point b, Point c) {
	int area =  (b.x - a.x) * (c.y - a.y) -
							(c.x - a.x) * (b.y - a.y);
	return area >= 0;
}

Point getFaceCentroid(Face *f) {
	int count = 1;
	Point p(-1,-1);

	if (f == NULL)
		return p;

	Edge *e = f->getEdge();
	Edge *iter = e->Lnext();

	p = e->Orig()->p;

	while (iter != e) {
		Point np = iter->Orig()->p;
		p += np;
		count++;
		iter = iter->Lnext();
	}

	p.x /= count;
	p.y /= count;

	return p;
}

Edge *getVerticesEdge(Vertex *v1, Vertex *v2) {
	if (v1 == NULL || v2 == NULL)
		return NULL;

	Edge *e = v1->getEdge();
	while (e->Dest() != v2) {
		e = e->Onext();
		if (e == v1->getEdge()) {
			e = NULL;
			break;
		}
	}
	return e;
}

int getVertexOrder(Edge *e) {
	int count = 1;
	Edge *iter = e->Onext();

	if (iter == NULL) {
		cout << "Erro: ponteiro next == NULL\n";
		return -1;
	}

	while (iter != e && count < 100) {
		count++;
		iter = iter->Onext();
	}

	if (count == 100) {
		cout << "Erro: loop infinito\n";
		return -1;
	}

	return count;
}

int getVertexOrder(Edge *e, vector<Point>& pts) {
	int count = 1;
	Edge *iter = e->Onext();

	if (iter == NULL) {
		cout << "Erro: ponteiro next == NULL\n";
		return -1;
	}

	pts.clear();
	pts.push_back(e->Sym()->Orig()->p);

	while (iter != e && count < 100) {
		count++;
		iter = iter->Onext();
		pts.push_back(iter->Sym()->Orig()->p);
	}

	if (count == 100) {
		cout << "Erro: loop infinito\n";
		return -1;
	}

	return count;
}

int getFaceOrder(Edge *e) {
	int count = 1;
	Edge *iter = e->Lnext();

	if (iter->Left() == NULL) {
		return -1;
	}
	if (iter == NULL) {
		cout << "Erro: ponteiro next == NULL\n";
		return -1;
	}

	while (iter != e && count < 100) {
		count++;
		iter = iter->Lnext();
	}

	if (count == 100) {
		cout << "Erro: loop infinito\n";
		return -1;
	}

	return count;
}

int getFaceOrder(Edge *e, vector<Vertex>& pts) {
	int count = 1;
	Edge *iter = e->Lnext();

	if (iter->Left() == NULL) {
		return -1;
	}
	if (iter == NULL) {
		cout << "Erro: ponteiro next == NULL\n";
		return -1;
	}

	pts.clear();
	pts.push_back(*e->Orig());

	while (iter != e && count < 100) {
		count++;
		pts.push_back(*iter->Orig());
		iter = iter->Lnext();
	}

	if (count == 100) {
		cout << "Erro: loop infinito\n";
		return -1;
	}

	return count;
}


Point *getPointsFromVertexList(vector<Vertex>& vertexList) {
	int i;
	Point * pts = new Point[vertexList.size()];
	for (i = 0; i < vertexList.size(); i++) {
		pts[i] = vertexList[i].p;
	}
	return pts;
}


Edge * splitFace(Face *fl, Vertex *v1, Vertex *v2) {

	Face *fr = new Face();

	//cout << "  split vertices: " << v1->p << "-" << v2->p << endl;
	Edge *a = v1->getEdge();
	Edge *b = v2->getEdge();
	Edge *c = Edge::makeEdge(v1, v2, fl, fr);

	while (a->Left() != fl) { a = a->Onext(); }
	while (b->Left() != fl) { b = b->Onext(); }

	Edge::splice(a, c);
	Edge::splice(b, c->Sym());

	Edge *cIter = c->Lnext();
	while (cIter != c) {
		cIter->setLeft(fl);
		cIter = cIter->Lnext();
	}
	cIter = c->Rnext();
	while (cIter != c) {
		cIter->setRight(fr);
		cIter = cIter->Rnext();
	}

	return c;
}


void splitEdge(Edge* e, Vertex *v, Edge *&e1, Edge *&e2) {

	cout << "  split edge: e = " << e << ", v = " << v->p << endl;
	Vertex *v1 = e->Orig();
	Vertex *v2 = e->Dest();
	Face *fl = e->Left();
	Face *fr = e->Right();
	Edge *a = e->Lprev();
	Edge *d = e->Lnext();

	cout << "    before kill: e = (" << e << ") " << endl;
	Edge::killEdge(e);

	Edge *b = Edge::makeEdge(v1, v, fl, fr);
	Edge *c = Edge::makeEdge(v, v2, fl, fr);
	cout << "               : b = (" << b << ") " << endl;
	cout << "               : c = (" << c << ") " << endl;

	Edge::splice(a->Sym(), b);
	Edge::splice(b->Sym(), c);
	Edge::splice(c->Sym(), d);

	e1 = b;
	e2 = c;
	cout << "    final: e = (" << e << ") " << endl;
}


bool intersectEdges(Edge *a, Edge *b, Point *pi) {

	// resolver 'p + tr = q + us'
	// onde t = (q - p) x s / (r x s)
	// e    u = (q - p) x r / (r x s)
	Point P = a->Orig()->p;
	Point Q = b->Orig()->p;
	Point R = (a->Dest()->p) - P;
	Point S = (b->Dest()->p) - Q;

	Point QmP = Q - P;
	double QmPxS = QmP.cross(S);
	double QmPxR = QmP.cross(R);
	double RxS = R.cross(S);

	if (RxS != 0.0) {
		// as linhas se intersectam, vamos calcular se os segmentos tambem
		double t = QmPxS / RxS;
		double u = QmPxR / RxS;
		// se 0<=t<=1 e 0<=u<=1, os segmentos se encontram em 'p + tr = q + us'
		if ((t >= 0) && (t <= 1) && (u >= 0) && (u <= 1)) {
			int x = (int)(P.x + (t * R.x));
			int y = (int)(P.y + (t * R.y));
			*pi = Point(x,y);
			return true;
		}
	} else {
	//	 os segmentos sao colineares ou paralelos, vamos ignorar ;)
	}

	return false;
}
