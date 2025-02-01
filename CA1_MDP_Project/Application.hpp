#pragma once
#include "StateStack.hpp"
#include "Player.hpp"
#include "TextureHolder.hpp"
#include "FontID.hpp"
#include "SoundPlayer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class Application
{
public:
	Application();
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
	Player m_player;
	StateStack m_state_stack;

	struct Context
	{
		Context(sf::RenderWindow& window, FontHolder& fonts, SoundPlayer& sounds, TextureHolder& textures, Player& player)
			: window(&window), fonts(&fonts), sounds(&sounds), textures(&textures), player(&player) {
		}

		sf::RenderWindow* window;
		FontHolder* fonts;
		SoundPlayer* sounds;
		TextureHolder* textures;
		Player* player;
	};
};