#include "MenuState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Constants.hpp"
#include <SFML/Window/Event.hpp>
#include "FontID.hpp"

MenuState::MenuState(StateStack& stack, Context context)
	: State(stack, context)
	, m_selected_option(0)
{
	sf::Font& font = context.fonts->Get(FontID::kMain);
	sf::Vector2f window_size(context.window->getSize());

	m_options.emplace_back("Play", font, 24);
	m_options.emplace_back("Exit", font, 24);

	for (std::size_t i = 0; i < m_options.size(); ++i)
	{
		sf::Text& text = m_options[i];
		text.setPosition(0.5f * window_size.x, 0.4f * window_size.y + i * 30.f);
		Utility::CentreOrigin(text);
	}
	UpdateOptionText();
}

void MenuState::Draw()
{
	Context context = GetContext();
	context.window->draw(m_background_sprite);
	for (const sf::Text& text : m_options)
	{
		context.window->draw(text);
	}
}

bool MenuState::Update(sf::Time dt)
{
	return false;
}

bool MenuState::HandleEvent(const sf::Event& event)
{
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
			if (m_selected_option == Play)
			{
				RequestStackPop();
				RequestStackPush(StateID::kGame);
			}
			else if (m_selected_option == Exit)
			{
				RequestStackPop();
			}
		}
	}
	return false;
}

void MenuState::UpdateOptionText()
{
	for (sf::Text& text : m_options)
		text.setFillColor(sf::Color::White);
	m_options[m_selected_option].setFillColor(sf::Color::Red);
}