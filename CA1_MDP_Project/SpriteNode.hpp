#pragma once
#include "SceneNode.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>

class SpriteNode : public SceneNode
{
public:
    explicit SpriteNode(const sf::Texture& texture);
    SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect);

    void SetVelocity(float vx, float vy);
    void SetVelocity(sf::Vector2f velocity);
    sf::Vector2f GetVelocity() const;

private:
    virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void UpdateCurrent(sf::Time dt) override;

private:
    sf::Sprite m_sprite;
    sf::Vector2f m_velocity;  
};
