// Updated GameState.cpp to use dynamic aircraft ID

#include "GameState.hpp"
#include "Player.hpp"
#include "MissionStatus.hpp"

GameState::GameState(StateStack& stack, Context context)
    : State(stack, context)
    , m_world(*context.window, *context.fonts, *context.sounds, false)
    , m_player(nullptr, 0, context.keys1) // ID set after AddAircraft()
    , m_player_id(-1) // Initially invalid
{
    // Dynamically assign player ID
    static int next_id = 1; // Shared across instances if needed
    m_player_id = next_id++;

    m_world.AddAircraft(m_player_id);
    m_player = Player(nullptr, m_player_id, context.keys1);
    m_player.SetMissionStatus(MissionStatus::kMissionRunning);

    context.music->Play(MusicThemes::kMissionTheme);
}

void GameState::Draw()
{
    m_world.Draw();
}

bool GameState::Update(sf::Time dt)
{
    m_world.Update(dt);

    int alive_count = 0;
    int last_alive_id = -1;

    for (int id = 0; id < 100; ++id)
    {
        if (Aircraft* aircraft = m_world.GetAircraft(id))
        {
            if (!aircraft->IsMarkedForRemoval())
            {
                ++alive_count;
                last_alive_id = id;
            }
        }
    }

    if (alive_count == 0)
    {
        m_player.SetMissionStatus(MissionStatus::kMissionFailure);
        RequestStackPush(StateID::kGameOver);
    }
    else if (alive_count == 1 && last_alive_id == m_player_id)
    {
        m_player.SetMissionStatus(MissionStatus::kMissionSuccess);
        RequestStackPush(StateID::kMissionSuccess);
    }

    CommandQueue& commands = m_world.GetCommandQueue();
    m_player.HandleRealtimeInput(commands);
    return true;
}

bool GameState::HandleEvent(const sf::Event& event)
{
    CommandQueue& commands = m_world.GetCommandQueue();
    m_player.HandleEvent(event, commands);

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        RequestStackPush(StateID::kPause);
    }
    return true;
}
