#pragma once
#include "Command.hpp"
#include "ReceiverCategories.hpp"
#include <map>
#include <SFML/Window/Keyboard.hpp>

class CommandQueue;

class Player
{
public:
	enum class Action
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Fire,
		LaunchMissile,
		ActionCount
	};

	explicit Player(int playerID);
	void HandleRealtimeInput(CommandQueue& commands);
	void HandleEvent(const sf::Event& event, CommandQueue& commands);
	void AssignKey(Action action, sf::Keyboard::Key key);
	sf::Keyboard::Key GetAssignedKey(Action action) const;

private:
	void InitializeKeyBindings();
	void InitializeActions();

private:
	std::map<sf::Keyboard::Key, Action> m_key_binding;
	std::map<Action, Command> m_action_binding;
	int m_playerID;
};
