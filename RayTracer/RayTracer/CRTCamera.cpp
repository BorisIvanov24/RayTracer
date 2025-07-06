#define _USE_MATH_DEFINES

#include "CRTCamera.h"
#include <cmath>

void CRTCamera::pan(const float degrees)
{
	const float rads = degrees * (M_PI / 180.f);
	const CRTMatrix rotateAroundY(
		cosf(rads), 0.f, -sinf(rads),
		0.f,        1.f, 0.f,
		sinf(rads), 0.f, cosf(rads)
	);

	rotationMatrix = rotationMatrix * rotateAroundY;
}

void CRTCamera::tilt(const float degrees)
{
	const float rads = degrees * (M_PI / 180.f);
	const CRTMatrix rotateAroundX(
		1.f, 0.f,		 0.f,
		0.f, cosf(rads), -sinf(rads),
		0.f, sinf(rads), cosf(rads)
	);

	rotationMatrix = rotationMatrix * rotateAroundX;
}

void CRTCamera::roll(const float degrees)
{
	const float rads = degrees * (M_PI / 180.f);
	const CRTMatrix rotateAroundZ(
		cosf(rads), -sinf(rads), 0.f,
		sinf(rads),  cosf(rads), 0.f,
		0.f,		 0.f,	     1.f
	);

	rotationMatrix = rotationMatrix * rotateAroundZ;
}

void CRTCamera::panAroundTarget(const float degrees, const CRTVector& target)
{
	// Move camera around the target
	CRTVector toCamera = position - target;

	const float rads = degrees * (M_PI / 180.f);
	CRTMatrix rotateAroundY(
		cosf(rads), 0.f, -sinf(rads),
		0.f, 1.f, 0.f,
		sinf(rads), 0.f, cosf(rads)
	);

	CRTVector rotated = toCamera * rotateAroundY;
	position = target + rotated;

	//  Here's the fix: always look at the target
	CRTVector viewDir = target - position;
	viewDir.normalise();

	CRTVector up(0.f, 1.f, 0.f);
	CRTVector right = cross(up, viewDir);
	right.normalise();
	CRTVector newUp = cross(viewDir, right);

	rotationMatrix = CRTMatrix(
		right.getX(), newUp.getX(), viewDir.getX(),
		right.getY(), newUp.getY(), viewDir.getY(),
		right.getZ(), newUp.getZ(), viewDir.getZ()
	);
}


const CRTVector& CRTCamera::getPosition() const
{
	return position;
}

const CRTMatrix& CRTCamera::getRotationMatrix() const
{
	return rotationMatrix;
}
