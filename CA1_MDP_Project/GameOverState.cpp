#include "GameOverState.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "Player.hpp"
#include "Utility.hpp"

#include <iostream>

GameOverState::GameOverState(StateStack& stack, Context context)
    : State(stack, context)
    , m_game_over_text()
    , m_elapsed_time(sf::Time::Zero)
{
    sf::Font& font = context.fonts->Get(Font::kMain);
    sf::Vector2f window_size(context.window->getSize());

    m_game_over_text.setFont(font);


    MissionStatus player1Status = context.player1->GetMissionStatus();
    MissionStatus player2Status = context.player2->GetMissionStatus();

    // Debugging output to confirm values
    std::cout << "DEBUG: GameOverState Loaded - Player 1 Status: " << static_cast<int>(player1Status) << "\n";
    std::cout << "DEBUG: GameOverState Loaded - Player 2 Status: " << static_cast<int>(player2Status) << "\n";

    if (player1Status == MissionStatus::kMissionSuccess)
    {
        m_game_over_text.setString("Mission Success for Player 1!");
    }
    else if (player2Status == MissionStatus::kMissionSuccess)
    {
        m_game_over_text.setString("Mission Success for Player 2!");
    }
    else
    {
        m_game_over_text.setString("Mission Failure");
    }

    m_game_over_text.setCharacterSize(50);
    Utility::CentreOrigin(m_game_over_text);
    m_game_over_text.setPosition(0.5f * window_size.x, 0.4f * window_size.y);
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
    //Show gameover for 3 seconds and then return to the main menu
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
    return false;
}
