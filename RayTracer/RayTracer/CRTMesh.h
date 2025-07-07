#pragma once
#include <vector>
#include "Math/CRTVector.h"

class CRTMesh
{
public:

	void addVertex(const CRTVector& vertex);
	void addIndex(int index);

	void print() const;
	const std::vector<CRTVector>& getVertices() const;
	const std::vector<int>& getIndices() const;

private:
	std::vector<CRTVector> vertices;
	std::vector<int> indices;
};

