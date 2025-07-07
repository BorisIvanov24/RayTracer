#include "CRTMesh.h"
#include <iostream>

void CRTMesh::addVertex(const CRTVector& vertex)
{
	vertices.push_back(vertex);
}

void CRTMesh::addIndex(int index)
{
	indices.push_back(index);
}

void CRTMesh::print() const
{
	for (const auto& obj : vertices)
	{
		obj.print();
	}

	for (int i=0;i<indices.size();i++)
	{
		if (i % 3 == 0)
			std::cout << std::endl;

		std::cout << indices[i] << ' ';
	}
}

const std::vector<CRTVector>& CRTMesh::getVertices() const
{
	return vertices;
}

const std::vector<int>& CRTMesh::getIndices() const
{
	return indices;
}
