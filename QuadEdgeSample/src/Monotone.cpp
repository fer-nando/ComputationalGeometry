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

Monotone::Monotone(Mesh &mesh, Mat &img) {
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
	newEdges.clear();

	// cria uma lista de prioridades ordenada pela coordenada y
	do {
		Vertex *v = e->Orig();
		queue.push(v);
		e = e->Lnext();
	} while (e != e0);

	// itera sob os vertices em 'queue'
	while (!queue.empty()) {

		// pega o vertice com a maior prioridade (menor y)
		Vertex *v = queue.top();

		// identifica o tipo do vertice
		int type = findVertexType(*v, f);
		cout << "Vertice: x=" << v->p.x << "; y=" << v->p.y << "; type=" << verticeTypeName[type] << endl;

		if(visual) {
			Mat img = src_img.clone();
			showNewEdges(img);
			showTree(img);
			showVertex(img, *v, Scalar(0,0,255));
			imshow(iter_window, img);
			waitKey();
		}

		// chama a funcao especifica para cada tipo de vertice
		switch(type) {
		case START:
			handleStartVertex(*v,f); break;
		case END:
			handleEndVertex(*v,f); break;
		case SPLIT:
			handleSplitVertex(*v,f); break;
		case MERGE:
			handleMergeVertex(*v,f); break;
		case REGULAR:
			handleRegularVertex(*v,f); break;
		}

		set<Edge *,EdgeComp>::iterator iter;
		cout << endl << "Tree:" << endl;
		for (iter = tree.begin(); iter != tree.end(); iter++) {
			Edge *e = *iter;
			cout << "  " << e->Orig()->p << " " << e->Dest()->p << endl;
		}
		cout << endl;

		// remove o vertice atual da fila
		queue.pop();
	}

}

int Monotone::findVertexType(Vertex &v, Face &f) {

	Edge *e = v.getEdge(&f);
	Vertex v1, v2;

	v1 = *(e->Lprev()->Orig());
	v2 = *(e->Dest());

	Vec3d p1(v1.p.x, v1.p.y, 0);
	Vec3d p2(v.p.x, v.p.y, 0);
	Vec3d p3(v2.p.x, v2.p.y, 0);
	Vec3d a = p2-p1;
	Vec3d b = p3-p2;

	Vec3d crossValue = a.cross(b);
	double acosValue = acos( a.dot(b) / (norm(a) * norm(b)) ) * 180 / M_PI;
	double angle = 180.0 + sign(crossValue[2]) * acosValue;
	cout << "Angle: " << angle << endl;

	if (angle <= 180) {
		if (v1.p.y >= v.p.y && v2.p.y >= v.p.y) {
			v.setType(START);
		} else if (v1.p.y <= v.p.y && v2.p.y <= v.p.y) {
			v.setType(END);
		} else {
			v.setType(REGULAR);
		}
	} else {
		if (v1.p.y >= v.p.y && v2.p.y >= v.p.y) {
			v.setType(SPLIT);
		} else if (v1.p.y <= v.p.y && v2.p.y <= v.p.y) {
			v.setType(MERGE);
		} else {
			v.setType(REGULAR);
		}
	}

	return v.getType();
}

void Monotone::handleStartVertex(Vertex &v, Face &f) {
	Edge *e = v.getEdge(&f);
	tree.insert(e);
	cout << "  inserted = " << e->Orig()->p << " " << e->Dest()->p << endl;
	helper[e] = &v;
}

void Monotone::handleEndVertex(Vertex &v, Face &f) {
	Edge *e = v.getEdge(&f)->Lprev();
	Vertex *v2 = helper[e];
	if (v2->getType() == MERGE) {
		Edge *newEdge = splitFace(&f, &v, v2);
		newEdges.push_back(newEdge);
		if (newEdge->Left() == &f)
			mesh.faces.push_back(newEdge->Left());
		else
			mesh.faces.push_back(newEdge->Right());
	}
	tree.erase(e);
	helper.erase(e);
}

void Monotone::handleSplitVertex(Vertex &v, Face &f) {

	Edge *curr = v.getEdge(&f);
	Edge *leftEdge = findLeftEdge(v,f);

	Edge *newEdge = splitFace(&f, &v, helper[leftEdge]);
	newEdges.push_back(newEdge);
	if (newEdge->Left() == &f)
		mesh.faces.push_back(newEdge->Left());
	else
		mesh.faces.push_back(newEdge->Right());

	helper[leftEdge] = &v;

	tree.insert(curr);
	helper[curr] = &v;
}

void Monotone::handleMergeVertex(Vertex &v, Face &f) {

	Edge *prev = v.getEdge(&f)->Lprev();

	if (helper[prev]->getType() == MERGE) {
		Edge *newEdge = splitFace(&f, &v, helper[prev]);
		newEdges.push_back(newEdge);
		if (newEdge->Left() == &f)
			mesh.faces.push_back(newEdge->Left());
		else
			mesh.faces.push_back(newEdge->Right());
	}

	tree.erase(prev);
	helper.erase(prev);

	Edge *leftEdge = findLeftEdge(v,f);

	if (helper[leftEdge]->getType() == MERGE) {
		Edge *newEdge = splitFace(&f, &v, helper[leftEdge]);
		newEdges.push_back(newEdge);
		if (newEdge->Left() == &f)
			mesh.faces.push_back(newEdge->Left());
		else
			mesh.faces.push_back(newEdge->Right());
	}

	helper[leftEdge] = &v;

}

void Monotone::handleRegularVertex(Vertex &v, Face &f) {

	Edge *curr = v.getEdge(&f);
	Edge *prev = v.getEdge(&f)->Lprev();
	cout << "  curr=" << curr->Orig()->p << "-" << curr->Dest()->p << endl;
	cout << "  prev=" << prev->Orig()->p << "-" << prev->Dest()->p << endl;

	// testa se o vertice anterior esta a cima do vertice atual
	if (curr->Orig()->p.y >= prev->Orig()->p.y) { // sim, o poligono esta a direita de v
		cout << "  >> direita" << endl;
		if (helper[prev]->getType() == MERGE) {
			Edge *newEdge = splitFace(&f, &v, helper[prev]);
			newEdges.push_back(newEdge);
			if (newEdge->Left() == &f)
				mesh.faces.push_back(newEdge->Left());
			else
				mesh.faces.push_back(newEdge->Right());
		}
		tree.erase(prev);
		helper.erase(prev);
		tree.insert(curr);
		helper[curr] = &v;
	} else { // nao, o poligono esta a esquerda de v
		cout << "  >> esquerda" << endl;
		Edge *leftEdge = findLeftEdge(v,f);
		if (helper[leftEdge]->getType() == MERGE) {
			Edge *newEdge = splitFace(&f, &v, helper[leftEdge]);
			newEdges.push_back(newEdge);
			if (newEdge->Left() == &f)
				mesh.faces.push_back(newEdge->Left());
			else
				mesh.faces.push_back(newEdge->Right());
		}
		helper[leftEdge] = &v;
	}
}

Edge* Monotone::findLeftEdge(Vertex &v, Face &f) {
	Edge *vertexEdge = v.getEdge(&f);
	Edge *leftEdge = *tree.begin();
	set<Edge *,EdgeComp>::iterator iter;
	EdgeComp comp = tree.key_comp();
	cout << "Left edge:" << endl;
	for (iter = tree.begin(); iter != tree.end(); iter++) {
		Edge *e = *iter;
		cout << "  e = " << e->Orig()->p << " " << e->Dest()->p;
		if (comp(vertexEdge, e)) {
			cout << endl;
			break;
		} else {
			leftEdge = e;
			cout << "  <<" << endl;
		}
	}
	return leftEdge;
}

Edge* Monotone::findRightEdge(Vertex &v, Face &f) {
	Edge *vertexEdge = v.getEdge(&f);
	Edge *rightEdge = *tree.rbegin();
	set<Edge *,EdgeComp>::reverse_iterator iter;
	EdgeComp comp = tree.key_comp();
	cout << "Right edge:" << endl;
	for (iter = tree.rbegin(); iter != tree.rend(); iter++) {
		Edge *e = *iter;
		cout << "  e = " << e->Orig()->p << " " << e->Dest()->p;
		if (!comp(vertexEdge, e)) {
			cout << endl;
			break;
		} else {
			rightEdge = e;
			cout << "  <<" << endl;
		}
	}
	return rightEdge;
}

void Monotone::showVertex(Mat &img, Vertex &v, const Scalar &color) {
	cv::circle(img, v.p, 8, color, -1, CV_AA);
}

void Monotone::showTree(Mat &img) {
	set<Edge *,EdgeComp>::iterator iter;
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
	cout << "new edges:" << endl;
	for (i = 0; i < newEdges.size(); i++) {
		Edge *e = newEdges[i];
		// draw current edge
		Point p1 = e->Orig()->p;
		Point p2 = e->Dest()->p;
		cout << p1 << "-" << p2 << endl;
		cv::line(img, p1, p2, Scalar(255,0,0), 3, CV_AA);
	}
}

