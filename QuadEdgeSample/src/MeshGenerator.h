
#include <vector>
#include "Edge.h"


class Mesh {

public:

	static int generateMesh(const char *filename, Mesh &mesh, int size);
	static int generateMesh(Mesh &mesh, int size);

	vector<Vertex *> vertices;
	vector<Face *> faces;
	vector<Edge *> edges;
};

