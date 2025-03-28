//Ross - D00241095 | Josh - D00238448
#pragma once
#include "State.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class PauseState : public State
{
public:
	PauseState(StateStack& stack, Context context);
	PauseState(StateStack& stack, Context context, bool isNetworked);
	~PauseState();
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event) override;

private:
	sf::Sprite m_background_sprite;
	sf::Text m_paused_text;
	sf::Text m_instruction_text;
	sf::Text m_esc_text;
};

