#include <iostream>
#include <fstream>
#include "Renderer.h"
#include "CRTScene.h"

int main()
{
	CRTScene scene("Scenes/scene5_Lec9.crtscene");

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

	//Renderer::renderAnimationLight(scene, "dragonAnim/scene5_Lec9.ppm");
	//Renderer::renderScene(scene, "testImage.ppm");
	Renderer::renderSceneLight(scene, "scene5_Lec9.ppm");
	//Renderer::renderAnimation(scene, "anim.ppm");

}
