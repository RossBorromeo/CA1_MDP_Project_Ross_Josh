//Ross - D00241095 | Josh - D00238448
#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "ResourceIdentifiers.hpp"
#include <iostream>
#include "Application.hpp"

int main()
{
	//TextureHolder game_textures;
	try
	{
		Application app;
		app.Run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}
}