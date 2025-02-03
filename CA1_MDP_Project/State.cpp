#include "State.hpp"
#include "StateID.hpp"
#include "StateStack.hpp"

//added in for 2 player movement by Josh
State::Context::Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, Player& player1, Player& player2, MusicPlayer& music, SoundPlayer& sounds) : window(&window), textures(&textures), fonts(&fonts), player1(&player1), player2(&player2), music(&music), sounds(&sounds)
{
}

State::State(StateStack& stack, Context context) : m_stack(&stack), m_context(context)
{
}

State::~State()
{
}

void State::RequestStackPush(StateID state_id)
{
    m_stack->PushState(state_id);
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
