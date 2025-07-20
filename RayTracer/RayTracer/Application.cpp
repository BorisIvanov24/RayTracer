#include <iostream>
#include <fstream>
#include "Renderer.h"
#include "CRTScene.h"

int main()
{
	CRTScene scene("Scenes/scene3_Lec8.crtscene");

	/*scene.getCamera().getPosition().print(std::cout);
	scene.getCamera().getRotationMatrix().print();
	scene.getSettings().backgroundColor.print(std::cout);
	std::cout << scene.getSettings().imageWidth << std::endl;
	std::cout << scene.getSettings().imageHeight << std::endl;

	std::cout << scene.getObjects().size() << std::endl;

	for (int i = 0; i < scene.getObjects().size(); i++)
	{
		scene.getObjects()[i].print();
	}*/


	Renderer::renderScene(scene, "testImage.ppm");
	//Renderer::renderAnimation(scene, "anim.ppm");

}
