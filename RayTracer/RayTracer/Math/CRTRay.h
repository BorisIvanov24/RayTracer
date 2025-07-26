#pragma once
#include "CRTVector.h"

enum class CRTRayType
{
	INVALID,
	CAMERA, //Ray shot from camera
	SHADOW, //When shading diffuse material
	REFLECTION, //After reflective material is hit
	REFRACTIVE //After refractive material is hit
};

class CRTRay
{
public:
	CRTRay(const CRTVector& origin, const CRTVector& direction, int pathDepth, CRTRayType type);
	
	const CRTVector& getOrigin() const;
	const CRTVector& getDirection() const;
	int getPathDepth() const;
	CRTRayType getType() const;

private:
	CRTVector origin;
	CRTVector direction;
	int pathDepth;
	CRTRayType type;
};

