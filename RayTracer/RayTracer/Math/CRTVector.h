#pragma once
class CRTVector
{
public:
	CRTVector();
	CRTVector(float x, float y, float z);

	float length() const;

	void normalise();

	float getX() const;
	float getY() const;
	float getZ() const;

	friend CRTVector operator+(const CRTVector& lhs, const CRTVector& rhs);
	friend CRTVector operator-(const CRTVector& lhs, const CRTVector& rhs);


	friend CRTVector operator*(const CRTVector& vec, float scalar);
	friend CRTVector operator*(float scalar, const CRTVector& vec);

	friend CRTVector cross(const CRTVector& lhs, const CRTVector& rhs);
	friend float dot(const CRTVector& lhs, const CRTVector& rhs);

private:
	float x, y, z;
};

