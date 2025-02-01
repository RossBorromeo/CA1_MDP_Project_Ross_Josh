#pragma once
#include "Entity.hpp"
#include "PickupType.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>

class Aircraft;

class Pickup : public Entity
{
public:
	Pickup(PickupType type, const TextureHolder& textures);
	virtual unsigned int GetCategory() const override;
	virtual sf::FloatRect GetBoundingRect() const override;
	void Apply(Aircraft& player) const;

	static PickupType GetRandomPickup(); // New: Randomized power-ups from meteors

protected:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;


private:
	PickupType m_type;
	sf::Sprite m_sprite;
	const TextureHolder& m_textures;
};
