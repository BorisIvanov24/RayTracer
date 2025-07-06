#pragma once
#include "Math/CRTVector.h"
#include "Math/CRTMatrix.h"

class CRTCamera
{
public:
	void pan(const float degrees);
	void tilt(const float degrees);
	void roll(const float degrees);

	void panAroundTarget(const float degrees, const CRTVector& target);

	const CRTVector& getPosition() const;
	const CRTMatrix& getRotationMatrix() const;

private:
	CRTMatrix rotationMatrix;

	CRTVector position;
};

