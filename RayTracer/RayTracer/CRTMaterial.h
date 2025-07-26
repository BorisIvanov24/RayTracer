#pragma once
#include "Math/CRTVector.h"

enum class CRTMaterialType
{
	INVALID,
	DIFFUSE,
	REFLECTIVE,
	REFRACTIVE,
	CONSTANT
};

class CRTMaterial
{
public: 
	CRTMaterialType getType() const;
	const CRTVector& getAlbedo() const;
	bool isSmoothShading() const;
	float getIor() const;

	void setType(CRTMaterialType type);
	void setAlbedo(const CRTVector& albedo);
	void setSmoothShading(bool smoothShading);
	void setIor(float ior);

private:
	CRTMaterialType type;
	CRTVector albedo;
	bool smoothShading;
	float ior;
};

