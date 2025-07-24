#pragma once
#include "Math/CRTVector.h"

enum class MaterialType
{
	DIFFUSE,
	REFLECTIVE
};

class CRTMaterial
{
public: 
	MaterialType getType() const;
	const CRTVector& getAlbedo() const;
	bool isSmoothShading() const;

	void setType(MaterialType type);
	void setAlbedo(const CRTVector& albedo);
	void setSmoothShading(bool smoothShading);

private:
	MaterialType type;
	CRTVector albedo;
	bool smoothShading;
};

