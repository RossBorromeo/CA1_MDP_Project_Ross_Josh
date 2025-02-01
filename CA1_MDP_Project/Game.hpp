#pragma once
#include "World.hpp"
#include "Player.hpp"
#include "TextureHolder.hpp"
#include "FontID.hpp"
#include "SoundPlayer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class Game
{
public:
	Game();
	void Run();

private:
	void ProcessInput();
	void Update(sf::Time delta_time);
	void Render();

private:
	static const sf::Time kTimePerFrame;

	sf::RenderWindow m_window;
	TextureHolder m_textures;
	FontHolder m_fonts;
	SoundPlayer m_sounds;
	World m_world;
	Player m_player;
};