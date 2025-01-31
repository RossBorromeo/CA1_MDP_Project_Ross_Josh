#include "Aircraft.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures)
	: Entity(100) // Set initial hitpoints
	, m_type(type)
	, m_sprite(textures.Get(type == AircraftType::kEagle ? TextureID::kEagle : TextureID::kRaptor))
	, m_missile_ammo(10) // Default ammo
	, m_hitpoints(100) // Default HP
	, m_fire_rate_level(1.0f)
	, m_fire_spread_level(1.0f)
{
	Utility::CentreOrigin(m_sprite);
}

void Aircraft::Accelerate(float vx, float vy)
{
	SetVelocity(GetVelocity() + sf::Vector2f(vx, vy));
}

void Aircraft::Repair(int amount)
{
	m_hitpoints = std::min(m_hitpoints + amount, 100);
}

void Aircraft::CollectMissiles(int amount)
{
	m_missile_ammo += amount;
}

void Aircraft::IncreaseFireRate()
{
	m_fire_rate_level += 0.1f;
}

void Aircraft::IncreaseFireSpread()
{
	m_fire_spread_level += 0.1f;
}

unsigned int Aircraft::GetCategory() const
{
	return static_cast<int>(m_type == AircraftType::kEagle ? ReceiverCategories::kPlayer1 : ReceiverCategories::kPlayer2);
}

sf::FloatRect Aircraft::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void Aircraft::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	Entity::UpdateCurrent(dt, commands);
}

void Aircraft::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}
