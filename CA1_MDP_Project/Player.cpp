#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Aircraft.hpp"
#include <SFML/Window/Event.hpp>

Player::Player(int playerID)
	: m_playerID(playerID)
{
	InitializeKeyBindings();
	InitializeActions();
}

void Player::HandleRealtimeInput(CommandQueue& commands)
{
	for (const auto& pair : m_key_binding)
	{
		if (sf::Keyboard::isKeyPressed(pair.first))
		{
			commands.Push(m_action_binding[pair.second]);
		}
	}
}

void Player::HandleEvent(const sf::Event& event, CommandQueue& commands)
{
	if (event.type == sf::Event::KeyPressed)
	{
		auto found = m_key_binding.find(event.key.code);
		if (found != m_key_binding.end())
		{
			commands.Push(m_action_binding[found->second]);
		}
	}
}

void Player::AssignKey(Action action, sf::Keyboard::Key key)
{
	for (auto it = m_key_binding.begin(); it != m_key_binding.end();)
	{
		if (it->second == action)
			it = m_key_binding.erase(it);
		else
			++it;
	}
	m_key_binding[key] = action;
}

sf::Keyboard::Key Player::GetAssignedKey(Action action) const
{
	for (const auto& pair : m_key_binding)
	{
		if (pair.second == action)
			return pair.first;
	}
	return sf::Keyboard::Unknown;
}

void Player::InitializeKeyBindings()
{
	if (m_playerID == 1)
	{
		m_key_binding[sf::Keyboard::A] = Action::MoveLeft;
		m_key_binding[sf::Keyboard::D] = Action::MoveRight;
		m_key_binding[sf::Keyboard::W] = Action::MoveUp;
		m_key_binding[sf::Keyboard::S] = Action::MoveDown;
		m_key_binding[sf::Keyboard::Space] = Action::Fire;
		m_key_binding[sf::Keyboard::LShift] = Action::LaunchMissile;
	}
	else if (m_playerID == 2)
	{
		m_key_binding[sf::Keyboard::Left] = Action::MoveLeft;
		m_key_binding[sf::Keyboard::Right] = Action::MoveRight;
		m_key_binding[sf::Keyboard::Up] = Action::MoveUp;
		m_key_binding[sf::Keyboard::Down] = Action::MoveDown;
		m_key_binding[sf::Keyboard::Enter] = Action::Fire;
		m_key_binding[sf::Keyboard::RShift] = Action::LaunchMissile;
	}
}

void Player::InitializeActions()
{
	const float playerSpeed = 200.f;
	const float missileSpeed = 300.f;

	m_action_binding[Action::MoveLeft].action = [playerSpeed](SceneNode& node, sf::Time dt)
		{
			if (auto* aircraft = dynamic_cast<Aircraft*>(&node))
			{
				aircraft->Accelerate(-playerSpeed * dt.asSeconds(), 0.f);
			}
		};

	m_action_binding[Action::MoveRight].action = [playerSpeed](SceneNode& node, sf::Time dt)
		{
			if (auto* aircraft = dynamic_cast<Aircraft*>(&node))
			{
				aircraft->Accelerate(playerSpeed * dt.asSeconds(), 0.f);
			}
		};

	m_action_binding[Action::MoveUp].action = [playerSpeed](SceneNode& node, sf::Time dt)
		{
			if (auto* aircraft = dynamic_cast<Aircraft*>(&node))
			{
				aircraft->Accelerate(0.f, -playerSpeed * dt.asSeconds());
			}
		};

	m_action_binding[Action::MoveDown].action = [playerSpeed](SceneNode& node, sf::Time dt)
		{
			if (auto* aircraft = dynamic_cast<Aircraft*>(&node))
			{
				aircraft->Accelerate(0.f, playerSpeed * dt.asSeconds());
			}
		};
}
