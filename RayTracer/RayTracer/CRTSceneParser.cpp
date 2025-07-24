#include "CRTSceneParser.h"
#include <iostream>
#include <fstream>
using namespace rapidjson;

CRTMatrix CRTSceneParser::loadMatrix(const rapidjson::Value::ConstArray& arr)
{
	assert(arr.Size() == 9);

	CRTMatrix matrix(
		static_cast<float>(arr[0].GetDouble()), static_cast<float>(arr[1].GetDouble()), static_cast<float>(arr[2].GetDouble()),
		static_cast<float>(arr[3].GetDouble()), static_cast<float>(arr[4].GetDouble()), static_cast<float>(arr[5].GetDouble()),
		static_cast<float>(arr[6].GetDouble()), static_cast<float>(arr[7].GetDouble()), static_cast<float>(arr[8].GetDouble())
	);

	return matrix;
}

CRTVector CRTSceneParser::loadVector(const Value::ConstArray& arr, int startIndex)
{
	assert(arr.Size() % 3 == 0);
	assert(startIndex < arr.Size() - 2);

	CRTVector vec(
		static_cast<float>(arr[startIndex].GetDouble()),
		static_cast<float>(arr[startIndex + 1].GetDouble()),
		static_cast<float>(arr[startIndex + 2].GetDouble())
	);

	return vec;
}

void CRTSceneParser::parseSettings(const Document& doc, CRTScene& scene)
{
	const Value& settingsVal = doc.FindMember("settings")->value;
	if (!settingsVal.IsNull() && settingsVal.IsObject())
	{
		const Value& backgroundColorVal = settingsVal.FindMember("background_color")->value;
		assert(!backgroundColorVal.IsNull() && backgroundColorVal.IsArray());
		scene.settings.backgroundColor = loadVector(backgroundColorVal.GetArray(), 0);

		const Value& imgSettings = settingsVal.FindMember("image_settings")->value;
		assert(!imgSettings.IsNull());

		const Value& width = imgSettings.FindMember("width")->value;
		assert(!width.IsNull());
		scene.settings.imageWidth = width.GetInt();

		const Value& height = imgSettings.FindMember("height")->value;
		assert(!height.IsNull());
		scene.settings.imageHeight = height.GetInt();
	}

	//std::cout << scene.settings.imageWidth << ' ' << scene.settings.imageHeight << std::endl;
	//scene.settings.backgroundColor.print(std::cout);
}

void CRTSceneParser::parseCamera(const rapidjson::Document& doc, CRTScene& scene)
{
	const Value& cameraVal = doc.FindMember("camera")->value;
	if (!cameraVal.IsNull() && cameraVal.IsObject())
	{
		const Value& matrixVal = cameraVal.FindMember("matrix")->value;
		assert(!matrixVal.IsNull() && matrixVal.IsArray());
		scene.camera.setRotationMatrix(loadMatrix(matrixVal.GetArray()));

		const Value& positionVal = cameraVal.FindMember("position")->value;
		assert(!positionVal.IsNull() && positionVal.IsArray());
		scene.camera.setPosition(loadVector(positionVal.GetArray(), 0));
	}
	//scene.camera.getPosition().print(std::cout);
	//scene.camera.getRotationMatrix().print();
}

void CRTSceneParser::parseMesh(const rapidjson::Value& val, CRTScene& scene)
{
	CRTMesh mesh;

	const Value& verticesVal = val.FindMember("vertices")->value;
	if (!verticesVal.IsNull() && verticesVal.IsArray())
	{
		int verticesCount = verticesVal.GetArray().Size();

		for (int i = 0; i < verticesCount/3; i++)
		{
			CRTVector vertex = loadVector(verticesVal.GetArray(), i * 3);
			mesh.addVertex(vertex);
		}
	}

	const Value& indicesVal = val.FindMember("triangles")->value;
	if (!indicesVal.IsNull() && indicesVal.IsArray())
	{
		int indicesCount = indicesVal.GetArray().Size();

		for (int i = 0; i < indicesCount; i++)
		{
			int index = indicesVal.GetArray()[i].GetInt();
			mesh.addIndex(index);
		}
	}

	int materialIndex;
	const Value& materialVal = val.FindMember("material_index")->value;
	if (!materialVal.IsNull())
	{
		materialIndex = materialVal.GetInt();
	}

	mesh.setMaterialIndex(materialIndex);
	mesh.calculateVertexNormals();

	scene.geometryObjects.push_back(mesh); //possible std::move
}

void CRTSceneParser::parseObjects(const rapidjson::Document& doc, CRTScene& scene)
{
	const Value& objectsVal = doc.FindMember("objects")->value;
	if (!objectsVal.IsNull() && objectsVal.IsArray())
	{
		int objectsCount = objectsVal.GetArray().Size();

		for (int i = 0; i < objectsCount; i++)
		{
			const Value& mesh = objectsVal.GetArray()[i];
			parseMesh(mesh, scene);
		}
	}
}


void CRTSceneParser::parseLights(const rapidjson::Document& doc, CRTScene& scene)
{
	const Value& lightsVal = doc.FindMember("lights")->value;

	if (!lightsVal.IsNull() && lightsVal.IsArray())
	{
		int lightsCount = lightsVal.GetArray().Size();

		for (int i = 0; i < lightsCount; i++)
		{
			const Value& light = lightsVal.GetArray()[i];
			parseLight(light, scene);
		}
	}
}

void CRTSceneParser::parseLight(const rapidjson::Value& val, CRTScene& scene)
{
	CRTVector position;
	float intensity = 0.f;

	const Value& positionVal = val.FindMember("position")->value;
	if (!positionVal.IsNull() && positionVal.IsArray())
	{
		position = loadVector(positionVal.GetArray(), 0);
	}

	const Value& intensityVal = val.FindMember("intensity")->value;
	if (!intensityVal.IsNull())
	{
		intensity = intensityVal.GetFloat();
	}

	CRTLight light(position, intensity);

	/*std::cout << "light:\n";
	position.print(std::cout);
	std::cout << intensity << std::endl;*/

	scene.lights.push_back(light);
}

void CRTSceneParser::parseMaterials(const rapidjson::Document& doc, CRTScene& scene)
{
	const Value& materialsVal = doc.FindMember("materials")->value;

	if (!materialsVal.IsNull() && materialsVal.IsArray())
	{
		int materialsCount = materialsVal.GetArray().Size();

		for (int i = 0; i < materialsCount; i++)
		{
			const Value& material = materialsVal.GetArray()[i];
			parseMaterial(material, scene);
		}
	}
}

static MaterialType getMaterialTypeFromString(const std::string& str)
{
	if (str == "diffuse")
	{
		return MaterialType::DIFFUSE;
	}

	return MaterialType::REFLECTIVE;
}

void CRTSceneParser::parseMaterial(const rapidjson::Value& val, CRTScene& scene)
{
	MaterialType type;
	CRTVector albedo;
	bool smoothShading;

	const Value& albedoVal = val.FindMember("albedo")->value;
	if (!albedoVal.IsNull() && albedoVal.IsArray())
	{
		albedo = loadVector(albedoVal.GetArray(), 0);
	}

	const Value& smoothShadingVal = val.FindMember("smooth_shading")->value;
	if (!smoothShadingVal.IsNull())
	{
		smoothShading = smoothShadingVal.GetBool();
	}

	const Value& typeVal = val.FindMember("type")->value;
	if (!typeVal.IsNull())
	{
		type = getMaterialTypeFromString(typeVal.GetString());
	}

	/*std::cout << "albedo:\n ";
	albedo.print(std::cout);
	std::cout << "smooth_shading:\n";
	std::cout << smoothShading <<std::endl;
	std::cout << "type\n";
	std::cout << (int)type << std::endl;*/


	CRTMaterial material;
	material.setAlbedo(albedo);
	material.setSmoothShading(smoothShading);
	material.setType(type);

	scene.materials.push_back(material);
}

void CRTSceneParser::parseScene(const std::string& sceneFileName, CRTScene& scene)
{
	std::ifstream ifs(sceneFileName);
	assert(ifs.is_open());

	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);

	parseSettings(doc, scene);
	parseCamera(doc, scene);
	parseObjects(doc, scene);
	parseLights(doc, scene);
	parseMaterials(doc, scene);

	/*for (auto& obj : scene.geometryObjects)
	{
		obj.print();
	}*/
}
