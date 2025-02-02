//D00238448:Joshua Thompson
//d00241095:Ross Borromeo
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

