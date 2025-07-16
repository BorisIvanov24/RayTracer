#include "Renderer.h"
#include <random>
#include <algorithm> // for std::min, std::max
#include <fstream>
#include <iostream>

template <typename T>
T clamp(T value, T minVal, T maxVal) {
	return std::max(minVal, std::min(value, maxVal));
}

int scaleTo255(float value, float minVal, float maxVal) {
	float normalized = (value - minVal) / (maxVal - minVal);
	return static_cast<int>(clamp(normalized * 255.0f, 0.0f, 255.0f));
}

// Function to convert triangle to RGB color
CRTVector triangleToColor(const CRTVector& p1, const CRTVector& p2, const CRTVector& p3) {
	// Compute centroid
	float cx = (p1.getX() + p2.getX() + p3.getX()) / 3.0f;
	float cy = (p1.getY() + p2.getY() + p3.getY()) / 3.0f;
	float cz = (p1.getZ() + p2.getZ() + p3.getZ()) / 3.0f;

	// Map centroid to RGB (assume coordinate bounds for normalization)
	int r = scaleTo255(cx, -1.0f, 1.0f); // adjust min/max bounds if needed
	int g = scaleTo255(cy, -1.0f, 1.0f);
	int b = scaleTo255(cz, -1.0f, 1.0f);

	return CRTVector(r, g, b);
}

CRTVector genRandomColor()
{
	std::random_device rd;
	std::mt19937 gen(rd());  // Mersenne Twister RNG

	std::uniform_int_distribution<> distrib(0, 255);

	return CRTVector(distrib(gen), distrib(gen), distrib(gen));
}

void writePixel(std::ofstream& ofs, const CRTVector& color)
{
	ofs << (int)color.getX() << ' ' << (int)color.getY() << ' ' << (int)color.getZ() << '\t';
}

void Renderer::renderScene(const CRTScene& scene, const std::string& outputFile) 
{
	CRTVector triColor(255, 0, 0);

	int screenWidth = scene.getSettings().imageWidth;
	int screenHeight = scene.getSettings().imageHeight;

	screenWidth /= 2;
	screenHeight /= 2;

	std::ofstream ofs(outputFile);

	ofs << "P3\n";
	ofs << screenWidth << " " << screenHeight << "\n";
	ofs << 255 << "\n";

	for (int j = 0; j < screenHeight; j++)
	{
		if (j % (screenHeight / 100) == 0)
			std::cout << (j / (screenHeight / 100)) << "%" << std::endl;

		for (int i = 0; i < screenWidth; i++)
		{
			CRTRay ray = genRay(i, j, scene.getCamera(), screenWidth, screenHeight);

			if (interesect(ray, scene, triColor))
			{
				writePixel(ofs, triColor);
			}
			else
			{
				CRTVector backColor(scene.getSettings().backgroundColor.getX() * 255,
					scene.getSettings().backgroundColor.getY() * 255,
					scene.getSettings().backgroundColor.getZ() * 255);

				writePixel(ofs, backColor);
			}
		}
		ofs << "\n";
	}
	std::cout << "100%" << std::endl;
	ofs.close();
}

CRTRay Renderer::genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight)
{
	float xF, yF;
	xF = x + 0.5;
	yF = y + 0.5;

	xF /= imageWidth;
	yF /= imageHeight;

	xF = (2.0 * xF) - 1.0;
	yF = 1.0 - (2.0 * yF);

	xF *= ((float)imageWidth / imageHeight);

	CRTVector direction(xF, yF, -1);
	direction = direction * camera.getRotationMatrix();
	direction.normalise();
	return CRTRay(camera.getPosition(), direction);
}

bool Renderer::isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle) 
{
	/*point.print();
	triangle.getVertex(0).print();
	triangle.getVertex(1).print();
	triangle.getVertex(2).print();*/

	CRTVector V0P = point - triangle.getVertex(0);
	CRTVector E0 = triangle.getVertex(1) - triangle.getVertex(0);

	if (!(dot(triangle.getNormal(), cross(E0, V0P)) > 0))
		return false;

	CRTVector V1P = point - triangle.getVertex(1);
	CRTVector E1 = triangle.getVertex(2) - triangle.getVertex(1);

	if (!(dot(triangle.getNormal(), cross(E1, V1P)) > 0))
		return false;

	CRTVector V2P = point - triangle.getVertex(2);
	CRTVector E2 = triangle.getVertex(0) - triangle.getVertex(2);

	if (!(dot(triangle.getNormal(), cross(E2, V2P)) > 0))
		return false;

	return true;
}

bool Renderer::interesect(const CRTRay& ray, const CRTScene& scene, CRTVector& color) 
{
	
	//ray.getDirection().print();
	//ray.getOrigin().print();

	float minRpDist = -1.f;
	CRTVector finalColor;

	for (int i = 0; i < scene.getObjects().size(); i++)
	{
		for (int j = 0; j < scene.getObjects()[i].getIndices().size() / 3; j++)
		{
			CRTVector v0 = scene.getObjects()[i].getVertices()[scene.getObjects()[i].getIndices()[j * 3]];
			CRTVector v1 = scene.getObjects()[i].getVertices()[scene.getObjects()[i].getIndices()[j * 3 + 1]];
			CRTVector v2 = scene.getObjects()[i].getVertices()[scene.getObjects()[i].getIndices()[j * 3 + 2]];

			CRTTriangle triangle(v0, v1, v2);

			//is ray parallel to the plane
			if (abs(dot(ray.getDirection(), triangle.getNormal())) < 0.0001)
			{
				continue;
			}

			float rpDist = dot(triangle.getVertex(0) - ray.getOrigin(), triangle.getNormal());

			//Back-face culling
			if (rpDist >= 0)
			{
				continue;
			}

			float rProj = dot(ray.getDirection(), triangle.getNormal());

			float t = abs(rpDist) / abs(rProj);

			//std::cout << t << std::endl;

			CRTVector p = ray.getOrigin() + t * ray.getDirection();

			//p.print();

			if (isPointInTriangle(p, triangle))
			{
				//std::cout << "in triangle!!!" << std::endl;

				rpDist = -rpDist;
				if (minRpDist < 0)
				{
					minRpDist = rpDist;
					finalColor = triangleToColor(triangle.getVertex(0), triangle.getVertex(1), triangle.getVertex(2));
				}
				else
				{
					if (rpDist < minRpDist)
					{
						minRpDist = rpDist;
						finalColor = triangleToColor(triangle.getVertex(0), triangle.getVertex(1), triangle.getVertex(2));
					}
				}
			}

		}
	}

	if (minRpDist < 0)
	{
		return false;
	}

	color = finalColor;
	return true;
}
