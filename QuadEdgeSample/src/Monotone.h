/*
 * Monotone.h
 *
 *  Created on: Aug 7, 2015
 *      Author: fernando
 */

#ifndef MONOTONE_H_
#define MONOTONE_H_

#include "Edge.h"

class Monotone {

public:
	Monotone();
	virtual ~Monotone();

	void makeMonotone(Face * f);

private:
	void handleStartVertex(Vertex * v);
	void handleEndVertex(Vertex * v);
	void handleSplitVertex(Vertex * v);
	void handleMergeVertex(Vertex * v);
	void handleRegularVertex(Vertex * v);

};

#endif /* MONOTONE_H_ */

