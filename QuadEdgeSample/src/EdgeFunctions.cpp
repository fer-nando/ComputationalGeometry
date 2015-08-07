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

int getFaceOrder(Edge *e, vector<Point>& pts) {
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
	pts.push_back(e->Orig()->p);

	while (iter != e && count < 100) {
		count++;
		pts.push_back(iter->Orig()->p);
		iter = iter->Lnext();
	}

	if (count == 100) {
		cout << "Erro: loop infinito\n";
		return -1;
	}

	return count;
}


Point *getPointsFromVertexList(vector<Vertex *>& vertexList) {
	int i;
	Point * pts = new Point[vertexList.size()];
	for (i = 0; i < vertexList.size(); i++) {
		pts[i] = vertexList[i]->p;
	}
	return pts;
}
