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
#include "Monotone.h"

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

/// Global variables
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


/// Function Headers
void help();

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
 * @function main
 */
int main(int, char** argv) {

	/// create a window
	namedWindow(main_window, WINDOW_NORMAL);

	//set the callback function for any mouse event
	setMouseCallback(main_window, CallBackFunc, &goal);

	/// create an image
	src = Mat(size, size, CV_8UC3, Scalar(255, 255, 255));

	/// Generate a mesh
	cout << "generate" << endl;
	//int nok = Mesh::generateMesh(mesh, size);
	int nok = Mesh::generateMesh("mesh4.obj", mesh, size);
	cout << "pos generate: " << mesh.vertices.size() << ", " << mesh.faces.size()
			<< ", " << mesh.edges.size() << endl << endl;

	if (nok) {
		cout << "Error";
		exit(1);
	}

	/// Draw the mesh
	drawMesh(src, mesh.faces, line_width);
	imshow(main_window, src);

	tria = src.clone();

	/// Make monotone
	Monotone monotone(&mesh, tria);
	int numFaces = mesh.faces.size();
	for (int i = 0; i < numFaces; i++) {
		monotone.makeMonotone(mesh.faces[i]);
	}
	monotone.clearNewEdges();
	waitKey(0);

	/// Draw the mesh
	tria = Scalar(255, 255, 255);
	drawMesh(tria, mesh.faces, line_width);

	/// Triangulate
	numFaces = mesh.faces.size();
	for (int i = 0; i < numFaces; i++) {
		monotone.triangulate(mesh.faces[i]);
	}
	monotone.closeWindows(0);

	/// Draw the mesh
	tria = Scalar(255, 255, 255);
	drawMesh(tria, mesh.faces, line_width);

	namedWindow(iter_window, WINDOW_NORMAL);
	imshow(iter_window, tria);

	/// Choose a start point
	e = mesh.edges[0];
	bool e_en = true, vert_en = true, face_en = true;
	const int radius = 2 * line_width;

	/// PART 2

	int key;

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

/// Draw current face
		if (face_en) {
			int faceOrder = getFaceOrder(e, pts);
			if (faceOrder > 0) {
				/// Highlight the polygon
				drawPolygon(iter, getPointsFromVertexList(pts), faceOrder, line_width,
						highlight_face_color, default_edge_color, default_vertex_color);
			}
		}
		/// Draw current quadedge
		if (e_en) {
			Point p1 = e->Orig()->p;
			Point p2 = e->Dest()->p;
			arrowedLine(iter, p1, p2, highlight_next_color, line_width, CV_AA);
		}
		/// Draw current vertex
		if (vert_en) {
			circle(iter, e->Orig()->p, radius, highlight_vertex_color, -1, CV_AA);
			circle(iter, e->Dest()->p, radius, highlight_vertex_color, -1, CV_AA);
		}
		/// Tries to reach the goal point
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

		key = waitKey(100);
		//cout << (char) key << "(" << key << ") pressed" << endl;
	}

	return 0;
}


/**
 * @function help
 * @brief Indications of how to run this program and why is it for
 */
void help() {
	printf("\t Half-edge data structure sample\n ");
	printf("\t---------------------------------\n ");
	printf(" Usage: ./Half-edge \n");
}
