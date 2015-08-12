/*
 * MeshGenerator.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: fernando
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "EdgeFunctions.h"
#include "MeshGenerator.h"


using namespace std;
using namespace cv;


struct classcomp {
  bool operator() (const Vec2i& lhs, const Vec2i& rhs) const
  {
  	if (lhs[0] < rhs[0]) {
  		return false;
  	} else if (rhs[0] < lhs[0]) {
  		return true;
  	} else {
  		if (lhs[1] <= rhs[1])
  			return false;
  		else
  			return true;
  	}
  }
};


int Mesh::generateMesh(const char *filename, Mesh &mesh, const int size) {

	FILE *f;
	map<Vec2i,Edge*,classcomp> edgeMap;

	std::locale::global( std::locale( "en_US" ) );

	f = fopen(filename, "r");


	if (f == NULL) {
		perror ("Error opening file");
		return 1;
	}

	while (!feof(f)) {
		float fx, fy, fz;

		char c = fgetc(f);
		ungetc(c, f);

		if (c != 'v') {
			break;
		}

		fscanf(f, "v %f %f %f\n", &fx, &fy, &fz);
		fx *= size;
		fy *= size;

		printf("v %f %f %f\n", fx, fy, fz);
		Vertex *vertex = new Vertex((int)fx, (int)fy);
		mesh.vertices.push_back(vertex);

	}


	while (!feof(f)) {

		int i;
		char line[100];
		vector<int> vnums;
		vector<Vertex *> vlist;
		vector<Edge *> elist;
		pair<map<Vec2i,Edge*,classcomp>::iterator,bool> ret;
		map<Vec2i,Edge*,classcomp>::iterator it;

		char d = fgetc(f);
		printf("%c ", d);

		fgets(line, 100, f);

		if (d != 'f') {
			continue;
		}

		Face *face = new Face();
		mesh.faces.push_back(face);

		char * tok = strtok(line, " ");
		while (tok != NULL) {
			int v = atoi(tok) - 1;
			vnums.push_back(v);
			printf("%i ", v);
			tok = strtok(NULL, " ");
		}
		printf("\n");

		for (i = 0; i < (int) vnums.size(); i++) {
			int a = (i == 0) ? vnums[vnums.size()-1] : vnums[i-1];
			int b = vnums[i];
			Edge *e1;
			Vec2i key1(min(a,b),max(a,b));
			it = edgeMap.find(key1);
			if (it != edgeMap.end()) {
				e1 = it->second->Sym();
				e1->setLeft(face);
				cout << "Aresta ja existe " << e1->Orig()->p << "-" << e1->Dest()->p << " - key " << key1 << endl;
			} else {
				e1 = Edge::makeEdge(mesh.vertices[a], mesh.vertices[b], face, NULL);
				edgeMap[key1] = e1;
				mesh.edges.push_back(e1);
				cout << "Criada aresta " << e1->Orig()->p << "-" << e1->Dest()->p << " - key " << key1 << endl;
			}
			elist.push_back(e1);
		}

		for (i = 0; i < (int) elist.size(); i++) {
			Edge *e1 = (i == 0) ? elist[elist.size()-1] : elist[i-1];
			Edge *e2 = elist[i];
			Edge::splice(e1->Sym()->Rnext()->Sym(), e2);
		}

	}

	fclose(f);

	return 0;
}


int Mesh::generateMesh(Mesh &mesh, const int size) {

	/// Vertices
	mesh.vertices.resize(8);
	mesh.vertices[0] = new Vertex(size/4, size/4);
	mesh.vertices[1] = new Vertex(size/4, size*3/4);
	mesh.vertices[2] = new Vertex(size*3/4, size*3/4);
	mesh.vertices[3] = new Vertex(size*3/4, size/4);

	mesh.vertices[4] = new Vertex(size/8, size/2);
	mesh.vertices[5] = new Vertex(size/2, size*7/8);
	mesh.vertices[6] = new Vertex(size*7/8, size/2);
	mesh.vertices[7] = new Vertex(size/2, size/8);

	/// Faces
	vector<Face *> faces = mesh.faces;
	mesh.faces.resize(6);
	mesh.faces[0] = new Face();

	mesh.faces[1] = new Face();
	mesh.faces[2] = new Face();
	mesh.faces[3] = new Face();
	mesh.faces[4] = new Face();

	mesh.faces[5] = new Face();

	/// Edges
	mesh.edges.resize(13);
	mesh.edges[0] = Edge::makeEdge(mesh.vertices[0], mesh.vertices[1], mesh.faces[0], mesh.faces[1]);
	mesh.edges[1] = Edge::makeEdge(mesh.vertices[1], mesh.vertices[2], mesh.faces[0], mesh.faces[2]);
	mesh.edges[2] = Edge::makeEdge(mesh.vertices[2], mesh.vertices[3], mesh.faces[5], mesh.faces[3]);
	mesh.edges[3] = Edge::makeEdge(mesh.vertices[3], mesh.vertices[0], mesh.faces[5], mesh.faces[4]);

	mesh.edges[4] = Edge::makeEdge(mesh.vertices[0], mesh.vertices[4], mesh.faces[1], NULL);
	mesh.edges[5] = Edge::makeEdge(mesh.vertices[4], mesh.vertices[1], mesh.faces[1], NULL);
	mesh.edges[6] = Edge::makeEdge(mesh.vertices[1], mesh.vertices[5], mesh.faces[2], NULL);
	mesh.edges[7] = Edge::makeEdge(mesh.vertices[5], mesh.vertices[2], mesh.faces[2], NULL);
	mesh.edges[8] = Edge::makeEdge(mesh.vertices[2], mesh.vertices[6], mesh.faces[3], NULL);
	mesh.edges[9] = Edge::makeEdge(mesh.vertices[6], mesh.vertices[3], mesh.faces[3], NULL);
	mesh.edges[10] = Edge::makeEdge(mesh.vertices[3], mesh.vertices[7], mesh.faces[4], NULL);
	mesh.edges[11] = Edge::makeEdge(mesh.vertices[7], mesh.vertices[0], mesh.faces[4], NULL);

	mesh.edges[12] = Edge::makeEdge(mesh.vertices[0], mesh.vertices[2], mesh.faces[0], mesh.faces[5]);

	/// Splice them all
	// square
	Edge::splice(mesh.edges[0]->Sym(), mesh.edges[1]);
	Edge::splice(mesh.edges[1]->Sym(), mesh.edges[2]);
	Edge::splice(mesh.edges[2]->Sym(), mesh.edges[3]);
	Edge::splice(mesh.edges[3]->Sym(), mesh.edges[0]);

	// tips: <, v, >, ^
	Edge::splice(mesh.edges[4]->Sym(), mesh.edges[5]);
	Edge::splice(mesh.edges[6]->Sym(), mesh.edges[7]);
	Edge::splice(mesh.edges[8]->Sym(), mesh.edges[9]);
	Edge::splice(mesh.edges[10]->Sym(), mesh.edges[11]);

	// link the square with tips
	Edge::splice(mesh.edges[3]->Sym(), mesh.edges[4]);
	Edge::splice(mesh.edges[5]->Sym(), mesh.edges[0]->Sym());

	Edge::splice(mesh.edges[5]->Sym(), mesh.edges[6]);
	Edge::splice(mesh.edges[7]->Sym(), mesh.edges[1]->Sym());

	Edge::splice(mesh.edges[7]->Sym(), mesh.edges[8]);
	Edge::splice(mesh.edges[9]->Sym(), mesh.edges[2]->Sym());

	Edge::splice(mesh.edges[9]->Sym(), mesh.edges[10]);
	Edge::splice(mesh.edges[11]->Sym(), mesh.edges[3]->Sym());

	Edge::splice(mesh.edges[0], mesh.edges[12]);
	Edge::splice(mesh.edges[2], mesh.edges[12]->Sym());

	return 0;

}



