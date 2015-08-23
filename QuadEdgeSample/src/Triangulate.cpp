/*
 * Triangulate.cpp
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
#include "Triangulate.h"
#include "EdgeFunctions.h"
#include "DrawFunctions.h"

#define _USE_MATH_DEFINES
#define sign(x) ((x >= 0.0 ? 1 : -1))

using namespace std;
using namespace cv;

enum Chains {
	UNDEF_CHAIN, LEFT_CHAIN, RIGHT_CHAIN
};

enum VerticeTypes {
	START, END, SPLIT, MERGE, REGULAR, UNDEF
};

const char * verticeTypeName[6] = { "START", "END", "SPLIT", "MERGE", "REGULAR",
		"UNDEF" };


/******************************************************************************
 ************************ Cosntructor & Destructor*****************************
 ******************************************************************************/

Triangulate::Triangulate(Mesh *mesh, Mat &img, const char *win) {
	this->mesh = mesh;
	src_img = img;
	if (!img.empty()) {
		visual = true;
	} else {
		visual = false;
	}
	iter_window = win;
}

Triangulate::~Triangulate() {
}

/******************************************************************************
 ************************ Monotone Functions **********************************
 ******************************************************************************/

void Triangulate::makeMonotone(Face *f) {

	Edge *e0, *e;
	Vertex *v;
	e0 = e = f->getEdge();

	cout << "## MakeMonotone start" << endl;

	// cria uma lista de prioridades ordenada pela coordenada y
	do {
		queue.push(e);
		e = e->Lnext();
	} while (e != e0);

	// itera sob os vertices em 'queue'
	while (!queue.empty()) {

		// pega o vertice com a maior prioridade (menor y)
		e = queue.top();
		v = e->Orig();

		// identifica o tipo do vertice
		int type = findVertexType(e);
		cout << "Vertice: x=" << v->p.x << "; y=" << v->p.y << "; type="
				<< verticeTypeName[type] << endl;

		if (visual) {
			img = src_img.clone();

			rectangle(img, Point(0, 0), Point(img.cols, v->p.y),
					Scalar(180, 180, 180), CV_FILLED);

			/*vector<Vertex> pts;
			int faceOrder = getFaceOrder(e, pts);
			if (faceOrder > 0)
				drawPolygon(img, getPointsFromVertexList(pts), faceOrder, line_width,
						highlight_face_color, default_edge_color, default_vertex_color);
			*/
			showNewEdges(img);
			showTree(img);
			showVertex(img, *v, Scalar(0, 0, 255));

			addWeighted(src_img, 0.1, img, 0.9, 0, img);

			imshow(iter_window, img);

			int key = waitKey(000);
			if (key == 27) {
				visual = false;
			}
		}

		// chama a funcao especifica para cada tipo de vertice
		switch (type) {
		case START:
			handleStartVertex(e);
			break;
		case END:
			handleEndVertex(e);
			break;
		case SPLIT:
			handleSplitVertex(e);
			break;
		case MERGE:
			handleMergeVertex(e);
			break;
		case REGULAR:
			handleRegularVertex(e);
			break;
		}

		// remove o vertice atual da fila
		queue.pop();
		cout << endl;
	}

	// limpa as estruturas de dados
	helper.clear();
	tree.clear();
	//newEdges.clear();

	cout << "## MakeMonotone end" << endl << endl;

}

int Triangulate::findVertexType(Edge *e) {

	Vertex *v1 = (e->Lprev()->Orig());
	Vertex *v2 = (e->Orig());
	Vertex *v3 = (e->Dest());

	double angle = getVertexAngle(v1, v2, v3);
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

void Triangulate::handleStartVertex(Edge *e) {
	tree.insert(e);
	//cout << "  inserted = " << e->Orig()->p << " " << e->Dest()->p << endl;
	helper[e] = e->Orig();
}

void Triangulate::handleEndVertex(Edge *e) {
	e = e->Lprev();
	Vertex *v1 = e->Orig();
	Vertex *v2 = helper[e];
	Face *f = e->Left();
	if (v2->getType() == MERGE) {
		insertNewEdge(f, v1, v2);

		if (visual) {
			showVertex(img, *v2, Scalar(0,0,255));
			drawDashedLine(img, v1->p, v2->p, Scalar(0,0,255), 10, CV_AA);
			imshow(iter_window, img);
			waitKey(000);
		}
	}
	tree.erase(e);
	helper.erase(e);
}

void Triangulate::handleSplitVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *leftEdge = findLeftEdge(e);

	insertNewEdge(f, v, helper[leftEdge]);

	if (visual) {
		Vertex *vh = helper[leftEdge];
		showVertex(img, *vh, Scalar(0,0,255));
		drawDashedLine(img, v->p, vh->p, Scalar(0,0,255), 10, CV_AA);
		imshow(iter_window, img);
		waitKey(000);
	}

	helper[leftEdge] = v;

	tree.insert(e);
	helper[e] = v;
}

void Triangulate::handleMergeVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *prev = e->Lprev();

	if (helper[prev]->getType() == MERGE) {
		insertNewEdge(f, v, helper[prev]);

		if (visual) {
			Vertex *vh = helper[prev];
			showVertex(img, *vh, Scalar(0,0,255));
			drawDashedLine(img, v->p, vh->p, Scalar(0,0,255), 10, CV_AA);
			imshow(iter_window, img);
			waitKey(000);
		}
	}

	tree.erase(prev);
	helper.erase(prev);

	Edge *leftEdge = findLeftEdge(e);

	if (helper[leftEdge]->getType() == MERGE) {
		insertNewEdge(f, v, helper[leftEdge]);

		if (visual) {
			Vertex *vh = helper[leftEdge];
			showVertex(img, *vh, Scalar(0,0,255));
			drawDashedLine(img, v->p, vh->p, Scalar(0,0,255), 10, CV_AA);
			imshow(iter_window, img);
			waitKey(000);
		}
	}

	helper[leftEdge] = v;

}

void Triangulate::handleRegularVertex(Edge *e) {

	Vertex *v = e->Orig();
	Face *f = e->Left();
	Edge *prev = e->Lprev();

	// testa se o vertice anterior esta a cima do vertice atual
	if (e->Orig()->p.y >= prev->Orig()->p.y) { // sim, o poligono esta a direita de v
		if (helper[prev]->getType() == MERGE) {
			insertNewEdge(f, v, helper[prev]);

			if (visual) {
				Vertex *vh = helper[prev];
				showVertex(img, *vh, Scalar(0,0,255));
				drawDashedLine(img, v->p, vh->p, Scalar(0,0,255), 10, CV_AA);
				imshow(iter_window, img);
				waitKey(000);
			}

		}
		tree.erase(prev);
		helper.erase(prev);
		tree.insert(e);
		helper[e] = v;

	} else { // nao, o poligono esta a esquerda de v

		Edge *leftEdge = findLeftEdge(e);

		if (helper[leftEdge]->getType() == MERGE) {
			insertNewEdge(f, v, helper[leftEdge]);

			if (visual) {
				Vertex *vh = helper[leftEdge];
				showVertex(img, *vh, Scalar(0,0,255));
				drawDashedLine(img, v->p, vh->p, Scalar(0,0,255), 10, CV_AA);
				imshow(iter_window, img);
				waitKey(000);
			}
		}
		helper[leftEdge] = v;
	}
}


/******************************************************************************
 *********************** Triangulate Functions ********************************
 ******************************************************************************/

void Triangulate::triangulate(Face *f) {

	int currChain = UNDEF_CHAIN;
	Edge *e, *lastE;
	Vertex *v;
	e = f->getEdge();

	cout << "## Triangulate start" << endl;

	// cria uma lista de prioridades ordenada pela coordenada y
	do {
		queue.push(e);
		e = e->Lnext();
	} while (e != f->getEdge());

	// remove o primeiro vertice e inicializa as cadeias esquerda e direita
	lastE = e = queue.top();
	queue.pop();
	chain.push_back(e);

	// itera sob os vertices em 'queue'
	while (!queue.empty()) {

		// pega o vertice com a maior prioridade (menor y)
		e = queue.top();
		v = e->Orig();

		// identifica a proxima aresta da mesma cadeia
		if (currChain == UNDEF_CHAIN) {
			currChain = (lastE == e->Lnext()) ? RIGHT_CHAIN : LEFT_CHAIN;
		}
		Edge *next = (currChain == RIGHT_CHAIN) ? lastE->Lprev() : lastE->Lnext();

		// define se o vertice esta na mesma cadeia ou do lado oposto
		if (e != next) { // caso 1
			cout << "Same chain" << endl;
			currChain = (currChain == RIGHT_CHAIN) ? LEFT_CHAIN : RIGHT_CHAIN;
			handleOppositeChain(e, lastE, currChain);
		} else { // caso 2
			cout << "Opposite chain" << endl;
			handleSameChain(e, lastE, currChain);
		}

		// se modo visual habilitado, atualiza a imagem
		if (visual) {
			img = src_img.clone();

			vector<Vertex> pts;
			int faceOrder = getFaceOrder(e, pts);
			if (faceOrder > 0)
				drawPolygon(img, getPointsFromVertexList(pts), faceOrder, line_width,
						highlight_face_color, default_edge_color, default_vertex_color);
			//rectangle(img, Point(0, 0), Point(img.cols, v->p.y),
			//		Scalar(180, 180, 180), CV_FILLED);

			showReflexChain(img, currChain);
			showNewEdges(img);
			showVertex(img, *v, Scalar(0, 0, 255));

			addWeighted(src_img, 0.1, img, 0.9, 0, img);

			imshow(iter_window, img);

			int key = waitKey(000);
			if (key == 27) {
				visual = false;
			}
		}

		cout << endl;
		lastE = e;
		queue.pop();

	}

	chain.clear();

	cout << "## Triangulate end" << endl << endl;

}


void Triangulate::handleSameChain(Edge *&e, Edge *&lastE, int currChain) {
	Vertex *v = e->Orig();

	// define o angulo do vertice atual
	Vertex *v1 = (lastE->Lprev()->Orig());
	Vertex *v2 = (lastE->Orig());
	Vertex *v3 = (lastE->Dest());
	double angle = getVertexAngle(v1, v2, v3);
	cout << "Angle: " << angle << endl;

	// verifica se o angulo do vertice anterior e menor que 180
	if (angle < 180) { // caso 2a

		Edge *newEdge = NULL;

		while (chain.size() > 1 && angle < 180) {

			Face *f = e->Left();

			chain.pop_back();
			lastE = chain.back();

			if (lastE != e->Lnext() && lastE != e->Lprev()) {

				newEdge = insertNewEdge(f, v, lastE->Orig());

				v1 = (lastE->Lprev()->Orig());
				v2 = (lastE->Orig());
				v3 = (lastE->Dest());
				angle = getVertexAngle(v1, v2, v3);
				cout << "  Angle: " << angle << endl;

				if (currChain == RIGHT_CHAIN) {
					e = newEdge;
				} else {
					chain.pop_back();
					chain.push_back(newEdge->Sym());
				}

			}
		}

		chain.push_back(e);

	} else { // caso 2b
		chain.push_back(e);
	}
}

void Triangulate::handleOppositeChain(Edge *&e, Edge *&lastE, int currChain) {
	Edge *newEdge = NULL;

	for (unsigned i = 1; i < chain.size(); i++) {

		Face *f = e->Left();
		Vertex *v1 = e->Orig();
		Vertex *v2 = chain[i]->Orig();

		newEdge = insertNewEdge(f, v1, v2);

		if (currChain == RIGHT_CHAIN) {
			e = newEdge;
		} else {
			lastE = newEdge->Sym();
		}

	}

	chain.clear();
	chain.push_back(lastE);
	chain.push_back(e);
}


/******************************************************************************
 ************************ Helper Functions ************************************
 ******************************************************************************/

double Triangulate::getVertexAngle(Vertex *v1, Vertex *v2, Vertex *v3) {
	Point a = v2->p - v1->p;
	Point b = v3->p - v2->p;

	double crossValue = a.cross(b);
	double acosValue = acos(a.dot(b) / (norm(a) * norm(b))) * 180 / M_PI;
	double angle = 180.0 + sign(crossValue) * acosValue;

	return angle;
}

Edge* Triangulate::findLeftEdge(Edge *vertexEdge) {

	Edge * leftEdge = NULL;
	vector<Edge *> leftEdges;
	set<Edge *, EdgeCompX>::iterator iter;

	// insere a aresta atual na arvore, sua posicao indicara o final das
	// das intersecoes
	pair<set<Edge *, EdgeCompX>::iterator,bool> end = tree.insert(vertexEdge);

	// cria uma linha horizontal na coordenada y
	int x = vertexEdge->Orig()->p.x;
	int y = vertexEdge->Orig()->p.y;
	Vertex v1(0,y), v2(x,y);
	Edge *horizontalLine = Edge::makeEdge(&v1, &v2, NULL, NULL);

	if (visual)
		line(img, v1.p, v2.p, Scalar(255, 0, 0), 3, CV_AA);

	// procura pela intersecao, a esquerda, com maior X
	Vertex v;
	Point pi, leftPi(0,y);
	for (iter = tree.begin(); iter != end.first; iter++) {
		Edge *e = *iter;
		bool intersect = intersectEdges(e, horizontalLine, &pi);
		if (intersect && pi.x > leftPi.x) {
			leftPi.x = pi.x;
			leftEdge = e;

			if (visual) {
				v.p.x = pi.x;
				v.p.y = pi.y;
				showVertex(img, v, Scalar(255, 0, 0));
			}
		}
	}

	if (visual) {
		v.p.x = leftPi.x;
		v.p.y = leftPi.y;
		showVertex(img, v, Scalar(0, 255, 255));
		imshow(iter_window, img);
		waitKey(0);
	}

	// remove a aresta da arvore
	tree.erase(end.first);

	// limpa memoria
	delete(horizontalLine);

	return leftEdge;
}

Edge* Triangulate::insertNewEdge(Face *f, Vertex *v1, Vertex *v2) {
	Edge *newEdge = splitFace(f, v1, v2);
	newEdges.push_back(newEdge);
	if (newEdge->Left() != f)
		newEdge = newEdge->Sym();
	mesh->faces.push_back(newEdge->Right());
	return newEdge;
}

void Triangulate::clearNewEdges() {
	newEdges.clear();
}

void Triangulate::closeWindows(int delay) {
	destroyWindow(iter_window);
	waitKey(delay);
}


/******************************************************************************
 ************************** Draw Functions ************************************
 ******************************************************************************/


void Triangulate::showVertex(Mat &img, Vertex &v, const Scalar &color) {
	circle(img, v.p, 10, color, -1, CV_AA);
}

void Triangulate::showTree(Mat &img) {
	set<Edge *, EdgeCompX>::iterator iter;
	for (iter = tree.begin(); iter != tree.end(); iter++) {
		Edge *e = *iter;
		// draw current edge
		Point p1 = e->Orig()->p;
		Point p2 = e->Dest()->p;
		line(img, p1, p2, Scalar(0, 255, 0), 4, CV_AA);
		// draw current edge helper
		Vertex *v = helper[e];
		showVertex(img, *v, Scalar(255, 0, 255));
		// draw a line to the helper
		if (p1 != v->p)
			arrowedLine(img, (p1+p2)*0.5, v->p, Scalar(0, 255, 0), 3, CV_AA);
	}
}

void Triangulate::showNewEdges(Mat &img) {
	unsigned i;

	for (i = 0; i < newEdges.size(); i++) {
		Edge *e = newEdges[i];
		// draw current edge
		Point p1 = e->Orig()->p;
		Point p2 = e->Dest()->p;
		drawDashedLine(img, p1, p2, Scalar(255, 0, 0), 10, CV_AA);
		//arrowedLine(img, p1, p2, Scalar(255, 0, 0), 3, CV_AA);
	}
}

void Triangulate::showReflexChain(Mat &img, int dir) {
	unsigned i;
	unsigned size = chain.size();
	const Scalar color = Scalar(0, 255, 0);

	for (i = 0; i < size; i++) {
		Edge *e = chain[i];
		// draw current vertex
		Vertex *v = e->Orig();
		showVertex(img, *v, color);
		// draw current edge
		Point p1 = v->p;
		Point p2 = e->Dest()->p;
		if (!(dir == RIGHT_CHAIN && i == 0) && !(dir == LEFT_CHAIN && i == size-1))
			line(img, p1, p2, color, 3, CV_AA);
	}
}
