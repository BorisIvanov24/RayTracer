#include <iostream>
#include <fstream>
#include "Renderer.h"
#include "CRTScene.h"

int main()
{
	CRTScene scene("Scenes/scene3.crtscene");
	Renderer::renderScene(scene, "testImage.ppm");
}
