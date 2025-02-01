#pragma once
#include "Entity.hpp"
#include "MeteorType.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>

class Meteor : public Entity
{
public:
	explicit Meteor(MeteorType type, const TextureHolder& textures);
	void TakeDamage(int damage);
	bool IsPowerUp() const;

	unsigned int GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;
	void MarkForRemoval() override;

protected:
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	MeteorType m_type;
	sf::Sprite m_sprite;
	const TextureHolder& m_textures;
	int m_hitpoints;
};
