#include <iostream>
#include <fstream>
#include "Renderer.h"
#include "CRTScene.h"

int main()
{
	CRTScene scene("Scenes/scene4_Lec12.crtscene");

	Renderer renderer(&scene);

	renderer.renderScene("scene4_Lec12.ppm");

}
