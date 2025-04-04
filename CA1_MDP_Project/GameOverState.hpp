//Ross - D00241095 | Josh - D00238448
#pragma once
#include "State.hpp"
#include <SFML/Graphics/Text.hpp>

class GameOverState : public State
{
public:
	GameOverState(StateStack& stack, Context context, const std::string& text);
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event);
	static std::string s_last_message;


private:
	sf::Text m_game_over_text;
	sf::Time m_elapsed_time;
};

