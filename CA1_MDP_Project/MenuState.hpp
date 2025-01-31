#pragma once
#include "State.hpp"
#include <vector>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

class MenuState : public State
{
public:
	MenuState(StateStack& stack, Context context);

	void Draw() override;
	bool Update(sf::Time dt) override;
	bool HandleEvent(const sf::Event& event) override;

private:
	void UpdateOptionText();

private:
	enum OptionNames { Play, Exit };

	sf::Sprite m_background_sprite;
	std::vector<sf::Text> m_options;
	std::size_t m_selected_option;
};