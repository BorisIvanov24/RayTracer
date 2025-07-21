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
};

class Renderer
{
public:
	static void renderScene(const CRTScene& scene, const std::string& outputFile);
	static void renderSceneLight(const CRTScene& scene, const std::string& outputFile);
	static void renderAnimation(const CRTScene& scene, const std::string& outputFile);
	static void renderAnimationLight(const CRTScene& scene, const std::string& outputFile);

private:
	static CRTRay genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight);
	static bool intersect(const CRTRay& ray, const CRTScene& scene, CRTVector& color, int i, int j);
	static bool isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle);
	static RayIntersectionData traceRay(const CRTRay& ray, const CRTScene& scene);
	static CRTVector handleShadowRays(const RayIntersectionData& data, const CRTScene& scene);
};

