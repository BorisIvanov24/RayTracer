#pragma once
#include <string>
#include "CRTScene.h"
#include "Math/CRTRay.h"
#include "Math/CRTTriangle.h"

struct RayIntersectionData
{
	bool isIntersected;
	CRTVector intersectionPoint;
	CRTTriangle intersectionTriangle;
	CRTVector intersectionPointNormal;
	const CRTMaterial* material = nullptr;
	CRTVector color;
	int objectIdx = -1;
	int triangleIdx = -1;
};

struct MinData
{
	float t;
	CRTTriangle triangle;
	int idx0;
	int idx1;
	int idx2;
	const CRTMesh* mesh = nullptr;
};

class Renderer
{
public:
	Renderer(const CRTScene* scene);
	void renderAnimation(const std::string& outputFileBaseName) const;
	void renderScene(const std::string& outputFile) const;

	static const int MAX_RAY_DEPTH = 5;
private:
	const CRTScene* scene = nullptr;

	CRTRay genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight) const;

	RayIntersectionData traceRay(const CRTRay& ray, float maxT = std::numeric_limits<float>::infinity()) const;

	bool isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle) const;
	
	CRTVector calculatePointNormal(const CRTVector& point, const CRTMesh& mesh, int idx0, int idx1, int idx2) const;

	CRTVector shade(const CRTRay& ray, const RayIntersectionData& data) const;

	CRTVector shadeDiffuse(const CRTRay& ray, const RayIntersectionData& data) const;
	CRTVector shadeReflective(const CRTRay& ray, const RayIntersectionData& data) const;
	CRTVector shadeRefractive(const CRTRay& ray, const RayIntersectionData& data) const;
	CRTVector shadeConstant(const CRTRay& ray, const RayIntersectionData& data) const;
};

