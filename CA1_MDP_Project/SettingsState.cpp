#include "SettingsState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Constants.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
	, m_selected_option(0)
{
	sf::Font& font = context.fonts->Get(FontID::kMain);
	sf::Vector2f window_size(context.window->getSize());

	m_options.emplace_back("Move Left", font, 24);
	m_options.emplace_back("Move Right", font, 24);
	m_options.emplace_back("Move Up", font, 24);
	m_options.emplace_back("Move Down", font, 24);
	m_options.emplace_back("Fire", font, 24);
	m_options.emplace_back("Launch Missile", font, 24);

	for (std::size_t i = 0; i < m_options.size(); ++i)
	{
		sf::Text& text = m_options[i];
		text.setPosition(0.5f * window_size.x, 0.4f * window_size.y + i * 30.f);
		Utility::CentreOrigin(text);
	}
	UpdateOptionText();
}

void SettingsState::Draw()
{
	Context context = GetContext();
	for (const sf::Text& text : m_options)
	{
		context.window->draw(text);
	}
}

bool SettingsState::Update(sf::Time dt)
{
	return false;
}

bool SettingsState::HandleEvent(const sf::Event& event)
{
	Context context = GetContext();
	Player& player = *context.player;

	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Up)
		{
			if (m_selected_option > 0)
				--m_selected_option;
			else
				m_selected_option = m_options.size() - 1;
			UpdateOptionText();
		}
		else if (event.key.code == sf::Keyboard::Down)
		{
			m_selected_option = (m_selected_option + 1) % m_options.size();
			UpdateOptionText();
		}
		else if (event.key.code == sf::Keyboard::Return)
		{
			// Convert selected option to Player::Action
			Player::Action action = static_cast<Player::Action>(m_selected_option);
			context.window->setKeyRepeatEnabled(false);
		}
	}
	else if (event.type == sf::Event::KeyReleased)
	{
		// Assign new key to selected action
		Player::Action action = static_cast<Player::Action>(m_selected_option);
		player.AssignKey(action, event.key.code);
		context.window->setKeyRepeatEnabled(true);
	}
	return false;
}

void SettingsState::UpdateOptionText()
{
	for (sf::Text& text : m_options)
		text.setFillColor(sf::Color::White);
	m_options[m_selected_option].setFillColor(sf::Color::Red);
}