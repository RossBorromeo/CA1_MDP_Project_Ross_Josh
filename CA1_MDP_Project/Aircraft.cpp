#include "Aircraft.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures)
	: Entity(2) // Aircraft starts with 2 HP
	, m_type(type)
	, m_sprite(textures.Get(type == AircraftType::kEagle ? TextureID::kEagle : TextureID::kRaptor))
{
	Utility::CentreOrigin(m_sprite);
}

void Aircraft::TakeDamage(int damage)
{
	m_hitpoints -= damage;
	if (m_hitpoints <= 0)
	{
		MarkForRemoval();
	}
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
