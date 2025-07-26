#include "CRTRay.h"

CRTRay::CRTRay(const CRTVector& origin, const CRTVector& direction, int pathDepth, CRTRayType type) 
	: origin(origin), direction(direction), pathDepth(pathDepth), type(type)
{
}

const CRTVector& CRTRay::getOrigin() const
{
	return origin;
}

const CRTVector& CRTRay::getDirection() const
{
	return direction;
}

int CRTRay::getPathDepth() const
{
	return pathDepth;
}

CRTRayType CRTRay::getType() const
{
	return type;
}
