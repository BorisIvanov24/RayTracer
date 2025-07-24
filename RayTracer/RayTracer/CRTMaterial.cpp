#include "CRTMaterial.h"

MaterialType CRTMaterial::getType() const
{
    return type;
}

const CRTVector& CRTMaterial::getAlbedo() const
{
    return albedo;
}

bool CRTMaterial::isSmoothShading() const
{
    return smoothShading;
}

void CRTMaterial::setType(MaterialType type)
{
    this->type = type;
}

void CRTMaterial::setAlbedo(const CRTVector& albedo)
{
    this->albedo = albedo;
}

void CRTMaterial::setSmoothShading(bool smoothShading)
{
    this->smoothShading = smoothShading;
}
