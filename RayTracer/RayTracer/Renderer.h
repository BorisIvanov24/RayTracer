#pragma once
#include <string>
#include "CRTScene.h"
#include "Math/CRTRay.h"
#include "Math/CRTTriangle.h"

class Renderer
{
public:
	static void renderScene(const CRTScene& scene, const std::string& outputFile);
private:
	static CRTRay genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight);
	static bool interesect(const CRTRay& ray, const CRTScene& scene, CRTVector& color);
	static bool isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle);
};

