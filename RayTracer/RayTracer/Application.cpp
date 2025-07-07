#include <iostream>
#include <fstream>
#include "Math/CRTRay.h"
#include "Math/CRTTriangle.h"
#include "CRTCamera.h"
#include "CRTScene.h"
#include <random>
#include <algorithm> // for std::min, std::max

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

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

CRTVector genRandomColor()
{
	std::random_device rd;
	std::mt19937 gen(rd());  // Mersenne Twister RNG

	std::uniform_int_distribution<> distrib(0, 255);

	return CRTVector(distrib(gen), distrib(gen), distrib(gen));
}

CRTRay genRay(int i, int j, const CRTCamera& camera)
{
	float x, y;
	x = i + 0.5;
	y = j + 0.5;

	x /= SCREEN_WIDTH;
	y /= SCREEN_HEIGHT;

	x = (2.0 * x) - 1.0;
	y = 1.0 - (2.0 * y);

	x *= ((float)SCREEN_WIDTH / SCREEN_HEIGHT);

	CRTVector direction(x, y, -1);
	direction = direction * camera.getRotationMatrix();
	direction.normalise();
	return CRTRay(camera.getPosition(), direction);
}

bool isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle)
{

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

bool interesect(const CRTRay& ray, const CRTScene& scene, CRTVector& color)
{	
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
			CRTVector p = ray.getOrigin() + t * ray.getDirection();

			if (isPointInTriangle(p, triangle))
			{
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

void writePixel(std::ofstream& ofs, const CRTVector& color)
{
	ofs << (int)color.getX() << ' ' << (int)color.getY() << ' ' << (int)color.getZ()<<'\t';
}

int main()
{

	CRTScene scene("Scenes/scene4.crtscene");

	CRTVector triColor(255, 0, 0);

	SCREEN_WIDTH = scene.getSettings().imageWidth;
	SCREEN_HEIGHT = scene.getSettings().imageHeight;

	SCREEN_WIDTH /= 10;
	SCREEN_HEIGHT /= 10;
	
	std::ofstream ofs("scene4Smaller.ppm");

	ofs << "P3\n";
	ofs << SCREEN_WIDTH << " " << SCREEN_HEIGHT << "\n";
	ofs << 255 << "\n";

	for (int j = 0; j < SCREEN_HEIGHT; j++)
	{
		if (j % (SCREEN_HEIGHT / 100) == 0)
			std::cout << (j / (SCREEN_HEIGHT / 100)) << "%" << std::endl;

		for (int i = 0; i < SCREEN_WIDTH; i++)
		{
			CRTRay ray = genRay(i, j, scene.getCamera());

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


	/*CRTVector triColor(255, 0, 0);
	CRTVector backgroundColor(0, 255, 0);

	for (int t = 0; t < 9; t++)
	{
		CRTCamera camera;
		camera.panAroundTarget(40 * t, CRTVector(0.f, -0.58, -4.f));
		char buffer[] = "animationX.ppm";
		buffer[9] = '0' + t;
		std::ofstream ofs(buffer);

		ofs << "P3\n";
		ofs << SCREEN_WIDTH << " " << SCREEN_HEIGHT << "\n";
		ofs << 255 << "\n";

		std::cout << "Working on frame: " << t << std::endl;

		for (int j = 0; j < SCREEN_HEIGHT; j++)
		{
			for (int i = 0; i < SCREEN_WIDTH; i++)
			{
				CRTRay ray = genRay(i, j, camera);

				if (interesect(ray, tri))
				{
					writePixel(ofs, triColor);
				}
				else
				{
					writePixel(ofs, backgroundColor);
				}
			}
			ofs << "\n";
		}
		ofs.close();
	}*/

