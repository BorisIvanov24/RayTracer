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
	//scene.settings.backgroundColor.print();
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
	//scene.camera.getPosition().print();
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

	/*for (auto& obj : scene.geometryObjects)
	{
		obj.print();
	}*/
}
