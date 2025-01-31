#include "Pickup.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Aircraft.hpp"
#include <random>
#include "ReceiverCategories.hpp"

namespace
{
	std::default_random_engine RandomEngine;
	std::uniform_int_distribution<int> PickupDistribution(0, static_cast<int>(PickupType::kPickupCount) - 1);
}

Pickup::Pickup(PickupType type, const TextureHolder& textures)
	: Entity(1), m_type(type), m_textures(textures),
	m_sprite(textures.Get(type == PickupType::kHealth ? TextureID::kHealthRefill :
		(type == PickupType::kMissile ? TextureID::kMissileRefill :
			(type == PickupType::kFireRate ? TextureID::kFireRate : TextureID::kFireSpread))))
{
	Utility::CentreOrigin(m_sprite);
}

unsigned int Pickup::GetCategory() const
{
	return static_cast<int>(ReceiverCategories::kPickup);
}

sf::FloatRect Pickup::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void Pickup::Apply(Aircraft& player) const
{
	switch (m_type)
	{
	case PickupType::kHealth:
		player.Repair(25);
		break;
	case PickupType::kMissile:
		player.CollectMissiles(3);
		break;
	case PickupType::kFireRate:
		player.IncreaseFireRate();
		break;
	case PickupType::kFireSpread:
		player.IncreaseFireSpread();
		break;
	}
}

PickupType Pickup::GetRandomPickup()
{
	return static_cast<PickupType>(PickupDistribution(RandomEngine));
}

void Pickup::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	SetVelocity(0.f, 50.f * dt.asSeconds()); // Power-ups float down
}

void Pickup::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}