//Ross - D00241095 | Josh - D00238448
#include "SpriteNode.hpp"

SpriteNode::SpriteNode(const sf::Texture& texture)
{
    m_sprite.setTexture(texture);
}

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect)
{
    m_sprite.setTexture(texture);
    m_sprite.setTextureRect(textureRect); //Set the texture rectangle correctly
}

void SpriteNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform(); // Apply transformation of parent nodes
    target.draw(m_sprite, states); //Draw the sprite
}

