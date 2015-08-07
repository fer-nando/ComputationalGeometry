/*
 * Monotone.cpp
 *
 *  Created on: Aug 7, 2015
 *      Author: fernando
 */

#include "Monotone.h"

struct vertexComp {
  bool operator() (const Vertex& lhs, const Vertex& rhs) const
  {
  	if (lhs.p.y < rhs.p.y) {
  		return false;
  	} else if (rhs.p.y < lhs.p.y) {
  		return true;
  	} else {
  		if (lhs.p.x <= rhs.p.x)
  			return false;
  		else
  			return true;
  	}
  }
};


Monotone::Monotone() {
	// TODO Auto-generated constructor stub

}

Monotone::~Monotone() {
	// TODO Auto-generated destructor stub
}


void Monotone::makeMonotone(Face * f) {

}

void Monotone::handleStartVertex(Vertex * v) {

}

void Monotone::handleEndVertex(Vertex * v) {

}

void Monotone::handleSplitVertex(Vertex * v) {

}

void Monotone::handleMergeVertex(Vertex * v) {

}

void Monotone::handleRegularVertex(Vertex * v) {

}
