/*
 * QuadEdge.cpp
 *
 *  Created on: Jul 8, 2015
 *      Author: fernando
 */

#include "Edge.h"
#include <iostream>

using namespace std;


/**
 * QuadEdge class
 */
class QuadEdge {

public:
	QuadEdge() {

		e[0].r = 0;
		e[1].r = 1;
		e[2].r = 2;
		e[3].r = 3;

		e[0].next = e+0;
		e[1].next = e+3;
		e[2].next = e+2;
		e[3].next = e+1;
	}

	Edge e[4];

};


Edge *Edge::makeEdge() {
	return (new QuadEdge())->e;
}

Edge *Edge::makeEdge(Vertex *vs, Vertex *ve, Face *fl, Face *fr) {
	Edge *e = Edge::makeEdge();
	e->setOrig(vs);
	e->setDest(ve);
	e->setLeft(fl);
	e->setRight(fr);
	return e;
}

void Edge::killEdge(Edge *e) {
	Edge *f = e->Sym();
	if (e->Onext() != e) splice(e, e->Oprev());
	if (f->Onext() != f) splice(f, f->Oprev());
	delete (QuadEdge *)(e - e->r);
}

void Edge::splice(Edge *a, Edge *b) {
	Edge *temp;
	Edge *alpha = a->Onext()->Rot();
	Edge *beta  = b->Onext()->Rot();

	temp = a->Onext();
	a->next = b->Onext();
	b->next = temp;
	temp = alpha->Onext();
	alpha->next = beta->Onext();
	beta->next = temp;
}


Edge::Edge() {
	orig = NULL;
	left = NULL;
	next = NULL;
	r = 0;
}

Edge::~Edge() {
}

void Edge::setOrig(Vertex* orig) {
	this->orig = orig;
	orig->setEdge(this);
}

Vertex* Edge::Orig() const {
	return orig;
}

void Edge::setDest(Vertex* dest) {
	Sym()->orig = dest;
	dest->setEdge(Sym());
}

Vertex* Edge::Dest() {
	return Sym()->orig;
}

void Edge::setLeft(Face* left) {
	this->left = left;
	if (left != NULL) left->setEdge(this);
}

Face* Edge::Left() {
	return left;
}

void Edge::setRight(Face* right) {
	Sym()->left = right;
	if (right != NULL) right->setEdge(Sym());
}

Face* Edge::Right() {
	return Sym()->left;
}

Edge* Edge::Rot() {
	return (r < 3) ? this+1 : this-3;
}

Edge* Edge::InvRot() {
	return (r > 0) ? this-1 : this+3;
}

Edge* Edge::Sym() {
	return (r < 2) ? this+2 : this-2;
}

Edge* Edge::Onext() {
	return next;
}

Edge* Edge::Dnext() {
	return Sym()->Onext()->Sym();
}

Edge* Edge::Lnext() {
	return InvRot()->Onext()->Rot();
}

Edge* Edge::Rnext() {
	return Rot()->Onext()->InvRot();
}

Edge* Edge::Oprev() {
	return Rot()->Onext()->Rot();
}

Edge* Edge::Dprev() {
	return InvRot()->Onext()->InvRot();
}

Edge* Edge::Lprev() {
	return Onext()->Sym();
}

Edge* Edge::Rprev() {
	return Sym()->Onext();
}
