#pragma once
#include <string>
#include "CRTMesh.h"
#include "CRTCamera.h"
#include "CRTLight.h"
#include "CRTMaterial.h"

struct CRTSettings
{
	CRTVector backgroundColor;
	int imageWidth;
	int imageHeight;
};

class CRTScene
{
public:
	friend class CRTSceneParser;

	CRTScene(const std::string& sceneFileName);

	void parseSceneFile(const std::string& sceneFileName);
	const CRTSettings& getSettings() const;
	const CRTCamera& getCamera() const;
	const std::vector<CRTMesh>& getObjects() const;
	const std::vector<CRTLight>& getLights() const;
	const std::vector<CRTMaterial>& getMaterials() const;

private:
	std::vector<CRTMesh> geometryObjects;
	CRTCamera camera;
	CRTSettings settings;
	std::vector<CRTLight> lights;
	std::vector<CRTMaterial> materials;
	
};

