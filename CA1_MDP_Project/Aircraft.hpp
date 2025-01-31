#pragma once
#include "Entity.hpp"
#include "AircraftType.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include "ReceiverCategories.hpp"

class Aircraft : public Entity
{
public:
	explicit Aircraft(AircraftType type, const TextureHolder& textures);
	void Accelerate(float vx, float vy);
	void Repair(int amount);
	void CollectMissiles(int amount);
	void IncreaseFireRate();
	void IncreaseFireSpread();

	unsigned int GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;

protected:
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	AircraftType m_type;
	sf::Sprite m_sprite;
	int m_missile_ammo;
	int m_hitpoints;
	float m_fire_rate_level;
	float m_fire_spread_level;
};
