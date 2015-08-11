/*
 * Monotone.cpp
 *
 *  Created on: Aug 7, 2015
 *      Author: fernando
 */

#include <iostream>
#include <vector>
#include <cmath>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "Monotone.h"
#include "EdgeFunctions.h"
#include "DrawFunctions.h"

#define _USE_MATH_DEFINES
#define sign(x) ((x >= 0.0 ? 1 : -1))

using namespace std;


enum VerticeTypes {
	START,
	END,
	SPLIT,
	MERGE,
	REGULAR,
	UNDEF
};

const char * verticeTypeName[6] = {
		"START",
		"END",
		"SPLIT",
		"MERGE",
		"REGULAR",
		"UNDEF"
};

Monotone::Monotone(Mesh *mesh, Mat &img) {
	this->mesh = mesh;
	src_img = img;
	if (!img.empty()) {
		visual = true;
		namedWindow(iter_window, WINDOW_NORMAL);
	} else {
		visual = false;
	}
}

Monotone::~Monotone() {
	// TODO Auto-generated destructor stub
}


void Monotone::makeMonotone(Face &f) {

	Edge *e0, *e;
	e0 = e = f.getEdge();

	// initializa as estruturas de dados
	while (!queue.empty()) {
		queue.pop();
	}
	helper.clear();
	tree.clear();
	//newEdges.clear();

	// cria uma lista de prioridades ordenada pela coordenada y
	do {
		queue.push(e);
		e = e->Lnext();
	} while (e != e0);

	// itera sob os vertices em 'queue'
	while (!queue.empty()) {

		// pega o vertice com a maior prioridade (menor y)
		Edge *e = queue.top();
		Vertex *v = e->Orig();

		// identifica o tipo do vertice
		int type = findVertexType(e);
		cout << "Vertice: x=" << v->p.x << "; y=" << v->p.y << "; type=" << verticeTypeName[type] << endl;

		if(visual) {
			Mat img = src_img.clone();

			vector<Vertex> pts;
			int faceOrder = getFaceOrder(e, pts);
			if (faceOrder > 0)
				drawPolygon(img, getPointsFromVertexList(pts), faceOrder, line_width,
						highlight_face_color, default_edge_color, default_vertex_color);
			rectangle(img, Point(0,0), Point(img.cols, v->p.y), Scalar(180,180,180), CV_FILLED);

			showNewEdges(img);
			showTree(img);
			showVertex(img, *v, Scalar(0,0,255));

			addWeighted(src_img, 0.1, img, 0.9, 0, img);

			imshow(iter_window, img);
			waitKey(1000);
		}

		// chama a funcao especifica para cada tipo de vertice
		switch(type) {
		case START:
			handleStartVertex(e); break;
		case END:
			handleEndVertex(e); break;
		case SPLIT:
			handleSplitVertex(e); break;
		case MERGE:
			handleMergeVertex(e); break;
		case REGULAR:
			handleRegularVertex(e); break;
		}

		/*set<Edge *,EdgeCompX>::iterator iter;
		cout << endl << "Tree:" << endl;
		for (iter = tree.begin(); iter != tree.end(); iter++) {
			Edge *e = *iter;
			cout << "  " << e->Orig()->p << " " << e->Dest()->p << endl;
		}
		cout << endl;*/

		// remove o vertice atual da fila
		queue.pop();
	}

}

int Monotone::findVertexType(Edge *e) {

	Vertex *v1, *v2, *v3;

	v1 = (e->Lprev()->Orig());
	v2 = (e->Orig());
	v3 = (e->Dest());

	Vec3d p1(v1->p.x, v1->p.y, 0);
	Vec3d p2(v2->p.x, v2->p.y, 0);
	Vec3d p3(v3->p.x, v3->p.y, 0);
	Vec3d a = p2-p1;
	Vec3d b = p3-p2;

	Vec3d crossValue = a.cross(b);
	double acosValue = acos( a.dot(b) / (norm(a) * norm(b)) ) * 180 / M_PI;
	double angle = 180.0 + sign(crossValue[2]) * acosValue;
	cout << "Angle: " << angle << endl;

	if (angle <= 180) {
		if (v1->p.y >= v2->p.y && v3->p.y >= v2->p.y) {
			v2->setType(START);
		} else if (v1->p.y <= v2->p.y && v3->p.y <= v2->p.y) {
			v2->setType(END);
		} else {
			v2->setType(REGULAR);
		}
	} else {
		if (v1->p.y >= v2->p.y && v3->p.y >= v2->p.y) {
			v2->setType(SPLIT);
		} else if (v1->p.y <= v2->p.y && v3->p.y <= v2->p.y) {
			v2->setType(MERGE);
		} else {
			v2->setType(REGULAR);
		}
	}

	return v2->getType();
}

void Monotone::handleStartVertex(Edge *e) {
	tree.insert(e);
	//cout << "  inserted = " << e->Orig()->p << " " << e->Dest()->p << endl;
	helper[e] = e->Orig();
}

void Monotone::handleEndVertex(Edge *e) {
	e = e->Lprev();
	Vertex *v1 = e->Orig();
	Vertex *v2 = helper[e];
	Face *f = e->Left();
	if (v2->getType() == MERGE) {
		insertNewEdge(f, v1, v2);
	}
	tree.erase(e);
	helper.erase(e);
}

void Monotone::handleSplitVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *leftEdge = findLeftEdge(e);

	insertNewEdge(f, v, helper[leftEdge]);

	helper[leftEdge] = v;

	tree.insert(e);
	helper[e] = v;
}

void Monotone::handleMergeVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *prev = e->Lprev();

	if (helper[prev]->getType() == MERGE) {
		insertNewEdge(f, v, helper[prev]);
	}

	tree.erase(prev);
	helper.erase(prev);

	Edge *leftEdge = findLeftEdge(e);

	if (helper[leftEdge]->getType() == MERGE) {
		insertNewEdge(f, v, helper[leftEdge]);
	}

	helper[leftEdge] = v;

}

void Monotone::handleRegularVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *prev = e->Lprev();
	//cout << "  curr=" << e->Orig()->p << "-" << e->Dest()->p << endl;
	//cout << "  prev=" << prev->Orig()->p << "-" << prev->Dest()->p << endl;

	// testa se o vertice anterior esta a cima do vertice atual
	if (e->Orig()->p.y >= prev->Orig()->p.y) { // sim, o poligono esta a direita de v
		//cout << "  >> direita" << endl;
		if (helper[prev]->getType() == MERGE) {
			insertNewEdge(f, v, helper[prev]);
		}
		tree.erase(prev);
		helper.erase(prev);
		tree.insert(e);
		helper[e] = v;

	} else { // nao, o poligono esta a esquerda de v

		//cout << "  >> esquerda" << endl;
		Edge *leftEdge = findLeftEdge(e);
		if (helper[leftEdge]->getType() == MERGE) {
			//cout << "  split: " << v->p << "-" << helper[leftEdge]->p << endl;
			insertNewEdge(f, v, helper[leftEdge]);
		}
		helper[leftEdge] = v;
	}
}

Edge* Monotone::findLeftEdge(Edge *vertexEdge) {
	Edge *leftEdge = *tree.begin();
	set<Edge *,EdgeCompX>::iterator iter;
	EdgeCompX comp = tree.key_comp();
	//cout << "Left edge:" << endl;
	for (iter = tree.begin(); iter != tree.end(); iter++) {
		Edge *e = *iter;
		//cout << "  e = " << e->Orig()->p << " " << e->Dest()->p;
		if (comp(vertexEdge, e)) {
			//cout << endl;
			break;
		} else {
			leftEdge = e;
			//cout << "  <<" << endl;
		}
	}
	return leftEdge;
}

Edge* Monotone::findRightEdge(Edge *vertexEdge) {
	Edge *rightEdge = *tree.rbegin();
	set<Edge *,EdgeCompX>::reverse_iterator iter;
	EdgeCompX comp = tree.key_comp();
	//cout << "Right edge:" << endl;
	for (iter = tree.rbegin(); iter != tree.rend(); iter++) {
		Edge *e = *iter;
		//cout << "  e = " << e->Orig()->p << " " << e->Dest()->p;
		if (!comp(vertexEdge, e)) {
			//cout << endl;
			break;
		} else {
			rightEdge = e;
			//cout << "  <<" << endl;
		}
	}
	return rightEdge;
}

void Monotone::showVertex(Mat &img, Vertex &v, const Scalar &color) {
	cv::circle(img, v.p, 8, color, -1, CV_AA);
}

void Monotone::showTree(Mat &img) {
	set<Edge *,EdgeCompX>::iterator iter;
	for (iter = tree.begin(); iter != tree.end(); iter++) {
		Edge *e = *iter;
		// draw current edge
		Point p1 = e->Orig()->p;
		Point p2 = e->Dest()->p;
		cv::line(img, p1, p2, Scalar(0,255,0), 4, CV_AA);
		// draw current edge helper
		Vertex *v = helper[e];
		showVertex(img, *v, Scalar(0,255,0));
	}
}

void Monotone::showNewEdges(Mat &img) {
	unsigned i;
	//cout << "new edges:" << endl;
	for (i = 0; i < newEdges.size(); i++) {
		Edge *e = newEdges[i];
		// draw current edge
		Point p1 = e->Orig()->p;
		Point p2 = e->Dest()->p;
		//cout << p1 << "-" << p2 << endl;
		drawDashedLine(img, p1, p2, Scalar(255,0,0), 10, CV_AA);
	}
}


void Monotone::insertNewEdge(Face *f, Vertex *v1, Vertex *v2) {
	Edge *newEdge = splitFace(f, v1, v2);
	newEdges.push_back(newEdge);
	if (newEdge->Left() != f)
		mesh->faces.push_back(newEdge->Left());
	else
		mesh->faces.push_back(newEdge->Right());
}

