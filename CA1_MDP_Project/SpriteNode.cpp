//Ross - D00241095 | Josh - D00238448
#include "SpriteNode.hpp"

SpriteNode::SpriteNode(const sf::Texture& texture) :m_sprite(texture)
{
}

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect) :m_sprite(texture, textureRect)
{
}

void SpriteNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}
