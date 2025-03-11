//Ross - D00241095 | Josh - D00238448
#pragma once
#include "State.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include "Container.hpp"

class PauseState : public State
{
public:
    PauseState(StateStack& stack, Context context, bool lets_updates_through = false);
    ~PauseState();
    virtual void Draw() override;
    virtual bool Update(sf::Time dt) override;
    virtual bool HandleEvent(const sf::Event& event) override;

private:
    sf::Sprite m_background_sprite;
    sf::Text m_paused_text;
    gui::Container m_gui_container;
    bool m_lets_updates_through;
};
