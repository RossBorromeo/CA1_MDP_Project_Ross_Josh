#include "GameState.hpp"
#include "Player.hpp"
#include "MissionStatus.hpp"

#include <iostream>

GameState::GameState(StateStack& stack, Context context) : State(stack, context), m_world(*context.window, *context.fonts, *context.sounds), m_player1(*context.player1), m_player2(*context.player2)
{
    //Play the music
    context.music->Play(MusicThemes::kMissionTheme);
}

void GameState::Draw()
{
    m_world.Draw();
}

bool GameState::Update(sf::Time dt)
{
    m_world.Update(dt);

    //This was added by Josh to get different Win screens depending on specific scenarios
    if (m_world.HasPlayer1ReachedEnd())
    {
        std::cout << "DEBUG: Player 1 reached the end. Setting success.\n";
        m_player1.SetMissionStatus(MissionStatus::kMissionSuccess);
        m_player2.SetMissionStatus(MissionStatus::kMissionFailure);
        std::cout << "DEBUG: Player 1 Status = " << static_cast<int>(m_player1.GetMissionStatus()) << "\n";
        std::cout << "DEBUG: Player 2 Status = " << static_cast<int>(m_player2.GetMissionStatus()) << "\n";
        Context context = GetContext();
        context.player1->SetMissionStatus(m_player1.GetMissionStatus());
        context.player2->SetMissionStatus(m_player2.GetMissionStatus());

        RequestStackPush(StateID::kGameOver);

    }

    else if (m_world.HasPlayer2ReachedEnd())
    {
        std::cout << "DEBUG: Player 2 reached the end. Setting success.\n";
        m_player2.SetMissionStatus(MissionStatus::kMissionSuccess);
        m_player1.SetMissionStatus(MissionStatus::kMissionFailure);
        std::cout << "DEBUG: Player 1 Status = " << static_cast<int>(m_player1.GetMissionStatus()) << "\n";
        std::cout << "DEBUG: Player 2 Status = " << static_cast<int>(m_player2.GetMissionStatus()) << "\n";
        Context context = GetContext();
        context.player1->SetMissionStatus(m_player1.GetMissionStatus());
        context.player2->SetMissionStatus(m_player2.GetMissionStatus());

        RequestStackPush(StateID::kGameOver);

    }

    else if (!m_world.HasAlivePlayer1())
    {
        std::cout << "DEBUG: Player 1 is dead. Player 2 wins.\n";
        m_player1.SetMissionStatus(MissionStatus::kMissionFailure);
        m_player2.SetMissionStatus(MissionStatus::kMissionSuccess);
        std::cout << "DEBUG: Player 1 Status = " << static_cast<int>(m_player1.GetMissionStatus()) << "\n";
        std::cout << "DEBUG: Player 2 Status = " << static_cast<int>(m_player2.GetMissionStatus()) << "\n";
        Context context = GetContext();
        context.player1->SetMissionStatus(m_player1.GetMissionStatus());
        context.player2->SetMissionStatus(m_player2.GetMissionStatus());

        RequestStackPush(StateID::kGameOver);

    }

    else if (!m_world.HasAlivePlayer2())
    {
        std::cout << "DEBUG: Player 2 is dead. Player 1 wins.\n";
        m_player2.SetMissionStatus(MissionStatus::kMissionFailure);
        m_player1.SetMissionStatus(MissionStatus::kMissionSuccess);
        std::cout << "DEBUG: Player 1 Status = " << static_cast<int>(m_player1.GetMissionStatus()) << "\n";
        std::cout << "DEBUG: Player 2 Status = " << static_cast<int>(m_player2.GetMissionStatus()) << "\n";
        Context context = GetContext();
        context.player1->SetMissionStatus(m_player1.GetMissionStatus());
        context.player2->SetMissionStatus(m_player2.GetMissionStatus());

        RequestStackPush(StateID::kGameOver);

    }

    // Handle player input
    CommandQueue& commands = m_world.GetCommandQueue();
    m_player1.HandleRealTimeInput(commands);
    m_player2.HandleRealTimeInput(commands);

    return true;
}





bool GameState::HandleEvent(const sf::Event& event)
{
    CommandQueue& commands = m_world.GetCommandQueue();
    m_player1.HandleEvent(event, commands);
    m_player2.HandleEvent(event, commands);


    //Escape should bring up the pause menu
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        RequestStackPush(StateID::kPause);
    }
    return true;
}
