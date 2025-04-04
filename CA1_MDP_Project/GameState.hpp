//D00238448:Joshua Thompson
//d00241095:Ross Borromeo
#pragma once
#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"

class GameState : public State
{
public:
	GameState(StateStack& stack, Context context);
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event) override;

private:
	World m_world;
	Player m_player;
	int m_player_id;
};

