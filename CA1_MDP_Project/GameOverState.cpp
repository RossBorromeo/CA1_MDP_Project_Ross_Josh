//D00238448:Joshua Thompson
//D00241095:Ross Borromeo

#include "GameOverState.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "Player.hpp"
#include "Utility.hpp"

std::string GameOverState::s_last_message = "Ship Destroyed";


GameOverState::GameOverState(StateStack& stack, Context context, const std::string& text)
    : State(stack, context)
    , m_game_over_text()
    , m_elapsed_time(sf::Time::Zero)
{
    sf::Font& font = context.fonts->Get(Font::kMain);
    sf::Vector2f window_size(context.window->getSize());
    m_game_over_text.setString(s_last_message); // use the static message

    m_game_over_text.setFont(font);
    m_game_over_text.setString(text);

    m_game_over_text.setCharacterSize(70);
    Utility::CentreOrigin(m_game_over_text);
    m_game_over_text.setPosition(0.5f * window_size.x, 0.4 * window_size.y);

}

void GameOverState::Draw()
{
    sf::RenderWindow& window = *GetContext().window;
    window.setView(window.getDefaultView());

    //Create a dark semi-transparent background
    sf::RectangleShape background_shape;
    background_shape.setFillColor(sf::Color(0, 0, 0, 150));
    background_shape.setSize(window.getView().getSize());

    window.draw(background_shape);
    window.draw(m_game_over_text);
}

bool GameOverState::Update(sf::Time dt)
{
    m_elapsed_time += dt;
    return false; // Let underlying game state continue updating
}

bool GameOverState::HandleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        RequestStackClear();
        RequestStackPush(StateID::kMenu);
    }
    return false;
}
