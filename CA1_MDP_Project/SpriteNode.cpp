#include "SpriteNode.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

SpriteNode::SpriteNode(const TextureHolder& textures, TextureID id)
{
	m_sprite.setTexture(textures.Get(id));
}

void SpriteNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}