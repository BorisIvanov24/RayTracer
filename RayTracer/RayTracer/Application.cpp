#include <iostream>
#include <fstream>
#include "Math/CRTRay.h"
#include "Math/CRTTriangle.h"

const int SCREEN_WIDTH = 1920/2;
const int SCREEN_HEIGHT = 1080/2;

CRTRay genRay(int i, int j)
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
	direction.normalise();
	return CRTRay(CRTVector(0, 0, 0), direction);
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

bool interesect(const CRTRay& ray, const CRTTriangle& triangle)
{	
	//ray is parallel to the plane
	if (abs(dot(ray.getDirection(), triangle.getNormal())) < 0.00001)
	{
		return false;
	}

	float rpDist = dot(triangle.getVertex(0), triangle.getNormal());

	//Back-face culling
	if (rpDist >= 0)
	{
		return false;
	}

	float rProj = dot(ray.getDirection(), triangle.getNormal());

	float t = abs(rpDist) / abs(rProj);
	CRTVector p = t * ray.getDirection();

	return isPointInTriangle(p, triangle);
}

void writePixel(std::ofstream& ofs, const CRTVector& color)
{
	ofs << (int)color.getX() << ' ' << (int)color.getY() << ' ' << (int)color.getZ()<<'\t';
}

int main()
{
	std::ofstream ofs("t1.ppm");

	ofs << "P3\n";
	ofs << SCREEN_WIDTH << " " << SCREEN_HEIGHT << "\n";
	ofs << 255 << "\n";

	CRTTriangle tri(
	CRTVector(-1.75, -7.75, -6),
	CRTVector(1.75, -1.75, -6),
	CRTVector(0, 1.75, -6)
	);

	CRTVector triColor(255, 0, 0);
	CRTVector backgroundColor(0, 255, 0);

	for (int j = 0; j < SCREEN_HEIGHT; j++)
	{
		for (int i = 0; i < SCREEN_WIDTH; i++)
		{
			CRTRay ray = genRay(i, j);

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

}