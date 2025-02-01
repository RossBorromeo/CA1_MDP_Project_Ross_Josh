#include "Application.hpp"
#include "StateStack.hpp"
#include "Utility.hpp"
#include "GameState.hpp"
#include "MenuState.hpp"
#include "PauseState.hpp"
#include "SettingsState.hpp"
#include "GameOverState.hpp"
#include <SFML/Window/Event.hpp>

const sf::Time Application::kTimePerFrame = sf::seconds(1.f / 60.f);

Application::Application()
	: m_window(sf::VideoMode(640, 480), "SFML Application")
	, m_textures()
	, m_fonts()
	, m_sounds()
	, m_player(1) // Ensure Player is initialized with a valid ID
	, m_state_stack(State::Context(m_window, m_fonts, m_sounds, m_textures, m_player))
{
	m_fonts.Load(FontID::kMain, "Media/Fonts/Sansation.ttf");
	m_textures.Load(TextureID::kTitleScreen, "Media/Textures/TitleScreen.png");
	m_state_stack.PushState(StateID::kMenu);
}

void Application::Run()
{
	sf::Clock clock;
	sf::Time time_since_last_update = sf::Time::Zero;
	while (m_window.isOpen())
	{
		time_since_last_update += clock.restart();
		while (time_since_last_update > kTimePerFrame)
		{
			time_since_last_update -= kTimePerFrame;
			ProcessInput();
			Update(kTimePerFrame);
		}
		Render();
	}
}

void Application::ProcessInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		m_state_stack.HandleEvent(event);
		if (event.type == sf::Event::Closed)
			m_window.close();
	}
}

void Application::Update(sf::Time delta_time)
{
	m_state_stack.Update(delta_time);
}

void Application::Render()
{
	m_window.clear();
	m_state_stack.Draw();
	m_window.display();
}
