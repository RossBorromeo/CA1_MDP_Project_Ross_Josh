#pragma once
#include "State.hpp"
#include "Player.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <vector>

class SettingsState : public State
{
public:
	SettingsState(StateStack& stack, Context context);

	void Draw() override;
	bool Update(sf::Time dt) override;
	bool HandleEvent(const sf::Event& event) override;

private:
	void UpdateOptionText();

private:

	sf::Sprite m_background_sprite;
	std::vector<sf::Text> m_options;
	std::size_t m_selected_option;
};
