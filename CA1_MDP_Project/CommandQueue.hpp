//Ross - D00241095 | Josh - D00238448
#pragma once
#include "Command.hpp"
#include <queue>

class CommandQueue
{
public:
	void Push(const Command& command);
	Command Pop();
	bool IsEmpty() const;

private:
	std::queue<Command> m_queue;
};

