//============================================================================
// Name        : HalfedgeSample.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <iostream>
#include <stdio.h>
#include "Edge.h"
#include "MeshGenerator.h"
#include "EdgeFunctions.h"
#include "DrawFunctions.h"
#include "Triangulate.h"

using namespace cv;
using namespace std;


typedef struct goal_str {
	bool changed;
	bool reached;
	Point *dest;
	Edge *start;
	goal_str() {
		changed = false;
		reached = true;
		dest = new Point(-1, -1);
		start = NULL;
	}
} Goal;

// Variaveis globais
Mat src, tria, iter;
vector<Vertex> pts;
Goal goal;
Edge *e;
Mesh mesh;

const int size = 800;

const char* main_window = "Mesh original";
const char* iter_window = "Quad-edge iterativo";

const int esc_key = 27;
const int face_key = 'f';
const int edge_key = 'e';
const int vert_key = 'v';
const int order_key = 'o';
const int twin_key = 's';
const int next_key = 'a';
const int prev_key = 'd';


/**
 * Função de callback para eventos de mouse
 * @param event
 * @param x
 * @param y
 * @param flags
 * @param ptr
 */
void CallBackFunc(int event, int x, int y, int flags, void* ptr) {

	if (event == EVENT_LBUTTONDOWN) {
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y
				<< ")" << endl;
		Goal *g = (Goal *) ptr;
		g->dest->x = x;
		g->dest->y = y;
		g->changed = true;
		g->reached = false;
		g->start = e;
	}

}

/**
 * Função principal
 * @function main
 */
int main(int, char** argv) {

	// Cria uma janela
	namedWindow(main_window, WINDOW_NORMAL);
	namedWindow(iter_window, WINDOW_NORMAL);

	// Cria uma imagem sizexsize, 24-bit (8+8+8), em branco [255,255,255]
	src = Mat(size, size, CV_8UC3, Scalar(255, 255, 255));


	// PARTE 1 - Cria um mesh em uma estrutura quadedge

	// Gera um mesh a partir de um arquivo .obj
	cout << "generate" << endl;;
	int nok = Mesh::generateMesh("teste.obj", mesh, size);
	cout << "pos generate: " << mesh.vertices.size() << ", " << mesh.faces.size()
			<< ", " << mesh.edges.size() << endl << endl;

	if (nok) {
		cout << "Error";
		exit(1);
	}

	// Desenha o mesh e mostra na janela
	drawMesh(src, mesh.faces, line_width);
	imshow(main_window, src);


	// PARTE 2 - Triangularização

	// Copia a imagem do mesh
	tria = src.clone();

	Triangulate triangulate(&mesh, tria, iter_window);

	// Torna todos as faces do mesh monotonas
	int numFaces = mesh.faces.size();
	for (int i = 0; i < numFaces; i++) {
		triangulate.makeMonotone(mesh.faces[i]);
	}
	triangulate.clearNewEdges();
	waitKey(0);

	// Desenha o mesh monotono
	tria = Scalar(255, 255, 255);
	drawMesh(tria, mesh.faces, line_width);

	// Triangulariza todas as faces monotonas
	numFaces = mesh.faces.size();
	for (int i = 0; i < numFaces; i++) {
		triangulate.triangulate(mesh.faces[i]);
	}
	//triangulate.closeWindows(0);


	// PARTE 3 - Iteração com o polígono

	// Desenha o mesh
	tria = Scalar(255, 255, 255);
	drawMesh(tria, mesh.faces, line_width);
	imshow(iter_window, tria);

	// Seta a função de callback para eventos de mouse
	setMouseCallback(iter_window, CallBackFunc, &goal);

	// Escolhe um ponto inicial
	e = mesh.edges[0];
	bool e_en = true, vert_en = true, face_en = true;
	const int radius = 2 * line_width;
	int key;

	// Loop, enquanto nao for apertado ESC
	while (key != esc_key) {

		iter = tria.clone();

		switch (key) {
		case twin_key:
			e = e->Sym();
			break;
		case next_key:
			e = e->Lnext();
			break;
		case prev_key:
			e = e->Lprev();
			break;
		case 'z':
			e = e->Onext();
			break;
		case 'c':
			e = e->Rnext()->Sym();
			break;
		case edge_key:
			e_en = !e_en;
			break;
		case vert_key:
			vert_en = !vert_en;
			break;
		case face_key:
			face_en = !face_en;
			break;
		case order_key:
			cout << "face order = " << getFaceOrder(e) << endl;
			cout << "edge order = " << getVertexOrder(e) << endl << endl;
			break;
		}

//		cout << "half-edge: " << he << endl;
//		cout << "  twin = " << he->twin << endl;
//		cout << "  next = " << he->next << endl;
//		cout << "  prev = " << he->prev << endl;
//		cout << "  orig = " << he->orig->p << endl;

		// Desenha a face atual
		if (face_en) {
			int faceOrder = getFaceOrder(e, pts);
			if (faceOrder > 0) {
				// Destaca o polígono
				drawPolygon(iter, getPointsFromVertexList(pts), faceOrder, line_width,
						highlight_face_color, default_edge_color, default_vertex_color);
			}
		}
		// Desenha o quadedge atual
		if (e_en) {
			Point p1 = e->Orig()->p;
			Point p2 = e->Dest()->p;
			arrowedLine(iter, p1, p2, highlight_next_color, line_width, CV_AA);
		}
		// Desenha o vertice atual
		if (vert_en) {
			circle(iter, e->Orig()->p, radius, highlight_vertex_color, -1, CV_AA);
			circle(iter, e->Dest()->p, radius, highlight_vertex_color, -1, CV_AA);
		}
		// Tenta chegar no ponto de destino (clicado)
		if (goal.start != NULL) {

			circle(iter, *goal.dest, radius, highlight_vertex_color, -1, CV_AA);

			if (!goal.reached) {
				int lefton = leftOn(e->Orig()->p, e->Dest()->p, *goal.dest);
				//cout << "leftOn = " << lefton << endl;
				//cout << "start = " << goal.start << " -- current = " << e << endl;

				if (lefton) {
					e = e->Sym();
					goal.start = e;
					goal.changed = true;
				} else {
					if (!goal.changed && e == goal.start) {
						goal.reached = true;
					} else {
						if (e->Left() == NULL)
							e = e->Lprev();
						else
							e = e->Lnext();
					}
					goal.changed = false;
				}

			}
		}

		imshow(iter_window, iter);

		// Espera 100ms, ou um botao pressionado
		key = waitKey(100);
		//cout << (char) key << "(" << key << ") pressed" << endl;
	}

	return 0;
}

/*int main() {

	Vertex a1(0,0), b1(10,10), c1(1,5), d1(5,5); // intersectam
	Vertex a2(0,0), b2(10,10), c2(1,5), d2(8,9); // nao intersectam
	Vertex a3(0,0), b3(10,10), c3(1,0), d3(11,10); // paralelas
	Vertex a4(0,0), b4(10,10), c4(3,3), d4(15,15); // colineares

	int i;
	for (i = 0; i < 4; i++) {

		bool inter;
		Vertex a, b, c, d;
		Point pi;

		switch(i) {
		case 0:
			a=a1; b=b1; c=c1; d=d1; break;
		case 1:
			a=a2; b=b2; c=c2; d=d2; break;
		case 2:
			a=a3; b=b3; c=c3; d=d3; break;
		case 3:
			a=a4; b=b4; c=c4; d=d4; break;
		}

		Edge *e1 = Edge::makeEdge(&a,&b,NULL,NULL);
		Edge *e2 = Edge::makeEdge(&c,&d,NULL,NULL);

		inter = intersectEdges(e1, e2, &pi);

		cout << "[" << i << "] Intersect = " << inter;
		if (inter) {
			cout << " -> " << pi;
		}
		cout << endl;
	}

}*/

