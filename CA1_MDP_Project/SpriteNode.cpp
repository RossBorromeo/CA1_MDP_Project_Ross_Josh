// Ross - D00241095 | Josh - D00238448
#include "SpriteNode.hpp"

SpriteNode::SpriteNode(const sf::Texture& texture)
    : m_velocity(0.f, 0.f) // Initialize velocity to zero
{
    m_sprite.setTexture(texture);
}

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect)
    : m_velocity(0.f, 0.f) // Initialize velocity to zero
{
    m_sprite.setTexture(texture);
    m_sprite.setTextureRect(textureRect); // Set the texture rectangle correctly
}

void SpriteNode::SetVelocity(float vx, float vy)
{
    m_velocity = sf::Vector2f(vx, vy);
}

void SpriteNode::SetVelocity(sf::Vector2f velocity)
{
    m_velocity = velocity;
}

sf::Vector2f SpriteNode::GetVelocity() const
{
    return m_velocity;
}

void SpriteNode::UpdateCurrent(sf::Time dt)
{
    // Move the node based on velocity and elapsed time
    move(m_velocity * dt.asSeconds());
}

void SpriteNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform(); // Apply transformation of parent nodes
    target.draw(m_sprite, states); // Draw the sprite
}
