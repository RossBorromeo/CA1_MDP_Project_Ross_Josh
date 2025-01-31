#pragma once
#include "Entity.hpp"
#include "MeteorType.hpp"
#include "ResourceIdentifiers.hpp"

class Meteor : public Entity
{
public:
	Meteor(MeteorType type, const TextureHolder& textures);
	unsigned int GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;
	void MarkForRemoval() override;
	bool IsPowerUp() const;

protected:
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	MeteorType m_type;
	sf::Sprite m_sprite;
	TextureHolder m_textures;
};

