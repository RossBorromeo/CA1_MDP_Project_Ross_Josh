#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class SpriteNode : public SceneNode
{
public:
	explicit SpriteNode(const TextureHolder& textures, TextureID id);

private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Sprite m_sprite;
};
