#include "State.hpp"
#include "StateStack.hpp"

State::Context::Context(sf::RenderWindow& window, FontHolder& fonts, SoundPlayer& sounds, TextureHolder& textures, Player& player)
	: window(&window)
	, fonts(&fonts)
	, sounds(&sounds)
	, textures(&textures)
	, player(&player)
{
}

State::State(StateStack& stack, Context context)
	: m_stack(&stack)
	, m_context(context)
{
}

State::~State()
{
}

void State::RequestStackPush(StateID stateID)
{
	m_stack->PushState(stateID);
}

void State::RequestStackPop()
{
	m_stack->PopState();
}

void State::RequestStackClear()
{
	m_stack->ClearStack();
}

State::Context State::GetContext() const
{
	return m_context;
}
