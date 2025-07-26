#include <iostream>
#include <fstream>
#include "Renderer.h"
#include "CRTScene.h"

int main()
{
	CRTScene scene("Scenes/scene8_Lec11.crtscene");

	Renderer renderer(&scene);

	renderer.renderScene("scene8_Lec11.ppm");

}
