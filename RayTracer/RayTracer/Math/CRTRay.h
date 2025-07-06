#pragma once
#include "CRTVector.h"

class CRTRay
{
public:
	CRTRay(const CRTVector& origin, const CRTVector& direction);
	
	const CRTVector& getOrigin() const;
	const CRTVector& getDirection() const;

private:
	CRTVector origin;
	CRTVector direction;
};

