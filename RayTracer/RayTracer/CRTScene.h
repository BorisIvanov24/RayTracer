#pragma once
#include <string>
#include "CRTMesh.h"
#include "CRTCamera.h"

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

private:
	std::vector<CRTMesh> geometryObjects;
	CRTCamera camera;
	CRTSettings settings;
	
};

