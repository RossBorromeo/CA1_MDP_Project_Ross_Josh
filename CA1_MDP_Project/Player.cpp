#include "Player.hpp"
#include "ReceiverCategories.hpp"
#include "Aircraft.hpp"

#include <iostream>


struct AircraftMover
{
    AircraftMover(float vx, float vy) :velocity(vx, vy)
    {}
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        aircraft.Accelerate(velocity);
    }

    sf::Vector2f velocity;
};

Player::Player() : m_current_mission_status(MissionStatus::kMissionRunning)
{
    //Set initial key bindings
    m_key_binding[sf::Keyboard::A] = Action::kMoveLeft;
    m_key_binding[sf::Keyboard::D] = Action::kMoveRight;
    m_key_binding[sf::Keyboard::W] = Action::kMoveUp;
    m_key_binding[sf::Keyboard::S] = Action::kMoveDown;
    m_key_binding[sf::Keyboard::M] = Action::kMissileFire;
    m_key_binding[sf::Keyboard::Space] = Action::kBulletFire;

    m_key_binding[sf::Keyboard::Left] = Action::kMoveLeft1;
    m_key_binding[sf::Keyboard::Right] = Action::kMoveRight1;
    m_key_binding[sf::Keyboard::Up] = Action::kMoveUp1;
    m_key_binding[sf::Keyboard::Down] = Action::kMoveDown1;
    m_key_binding[sf::Keyboard::RControl] = Action::kMissileFire1;
    m_key_binding[sf::Keyboard::RShift] = Action::kBulletFire1;



    //Set initial action bindings
    InitialiseActions();

    m_action_binding[Action::kMoveLeft].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);
    m_action_binding[Action::kMoveRight].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);
    m_action_binding[Action::kMoveUp].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);
    m_action_binding[Action::kMoveDown].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);
    m_action_binding[Action::kBulletFire].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);
    m_action_binding[Action::kMissileFire].category = static_cast<int>(ReceiverCategories::kPlayerAircraft1);

    m_action_binding[Action::kMoveLeft1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);
    m_action_binding[Action::kMoveRight1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);
    m_action_binding[Action::kMoveUp1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);
    m_action_binding[Action::kMoveDown1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);
    m_action_binding[Action::kBulletFire1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);
    m_action_binding[Action::kMissileFire1].category = static_cast<int>(ReceiverCategories::kPlayerAircraft2);

}

void Player::HandleEvent(const sf::Event& event, CommandQueue& command_queue)
{
    if (event.type == sf::Event::KeyPressed)
    {
        auto found = m_key_binding.find(event.key.code);
        if (found != m_key_binding.end() && !IsRealTimeAction(found->second))
        {
            command_queue.Push(m_action_binding[found->second]);
        }
    }
}

void Player::HandleRealTimeInput(CommandQueue& command_queue)
{
    //Check if any of the key bindings are pressed
    for (auto pair : m_key_binding)
    {
        if (sf::Keyboard::isKeyPressed(pair.first) && IsRealTimeAction(pair.second))
        {
            command_queue.Push(m_action_binding[pair.second]);
        }
    }
}

void Player::AssignKey(Action action, sf::Keyboard::Key key)
{
    //Remove keys that are currently bound to the action
    for (auto itr = m_key_binding.begin(); itr != m_key_binding.end();)
    {
        if (itr->second == action)
        {
            m_key_binding.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
    m_key_binding[key] = action;
}

sf::Keyboard::Key Player::GetAssignedKey(Action action) const
{
    for (auto pair : m_key_binding)
    {
        if (pair.second == action)
        {
            return pair.first;
        }
    }
    return sf::Keyboard::Unknown;
}

void Player::SetMissionStatus(MissionStatus status)
{
    std::cout << "DEBUG: Setting mission status to " << static_cast<int>(status) << "\n";
    m_current_mission_status = status;
}


MissionStatus Player::GetMissionStatus() const
{
    return m_current_mission_status;
}

void Player::InitialiseActions()
{
    const float kPlayerSpeed = 200.f;
    m_action_binding[Action::kMoveLeft].action = DerivedAction<Aircraft>(AircraftMover(-kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveRight].action = DerivedAction<Aircraft>(AircraftMover(kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveUp].action = DerivedAction<Aircraft>(AircraftMover(0.f, -kPlayerSpeed));
    m_action_binding[Action::kMoveDown].action = DerivedAction<Aircraft>(AircraftMover(0.f, kPlayerSpeed));
    m_action_binding[Action::kBulletFire].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.Fire();
        }
    );

    m_action_binding[Action::kMissileFire].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.LaunchMissile();
        }
    );

    m_action_binding[Action::kMoveLeft1].action = DerivedAction<Aircraft>(AircraftMover(-kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveRight1].action = DerivedAction<Aircraft>(AircraftMover(kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveUp1].action = DerivedAction<Aircraft>(AircraftMover(0.f, -kPlayerSpeed));
    m_action_binding[Action::kMoveDown1].action = DerivedAction<Aircraft>(AircraftMover(0.f, kPlayerSpeed));
    m_action_binding[Action::kBulletFire1].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.Fire();
        }
    );

    m_action_binding[Action::kMissileFire1].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.LaunchMissile();
        }
    );
}




bool Player::IsRealTimeAction(Action action)
{
    switch (action)
    {
    case Action::kMoveLeft:
    case Action::kMoveRight:
    case Action::kMoveDown:
    case Action::kMoveUp:
    case Action::kBulletFire:
    case Action::kMoveLeft1:
    case Action::kMoveRight1:
    case Action::kMoveDown1:
    case Action::kMoveUp1:
    case Action::kBulletFire1:
        return true;
    default:
        return false;
    }
}

