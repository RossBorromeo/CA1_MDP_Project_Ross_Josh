#include "GameOverState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Constants.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include "FontID.hpp"

GameOverState::GameOverState(StateStack& stack, Context context)
	: State(stack, context)
	, m_elapsed_time(sf::Time::Zero)
{
	sf::Font& font = context.fonts->Get(FontID::kMain);
	sf::Vector2f window_size(context.window->getSize());

	m_game_over_text.setFont(font);
	m_game_over_text.setString("Game Over");
	m_game_over_text.setCharacterSize(40);
	Utility::CentreOrigin(m_game_over_text);
	m_game_over_text.setPosition(0.5f * window_size.x, 0.4f * window_size.y);
}

void GameOverState::Draw()
{
	Context context = GetContext();
	context.window->clear();
	context.window->draw(m_game_over_text);
}

bool GameOverState::Update(sf::Time dt)
{
	m_elapsed_time += dt;
	if (m_elapsed_time > sf::seconds(3))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return false;
}

bool GameOverState::HandleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return false;
}