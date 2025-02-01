#include "Meteor.hpp"
#include "MeteorType.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "Pickup.hpp"
#include "CommandQueue.hpp"
#include "SceneNode.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

Meteor::Meteor(MeteorType type, const TextureHolder& textures)
	: Entity(1), m_type(type), m_sprite(textures.Get(type == MeteorType::kPowerUp ? TextureID::kPowerupMeteor : TextureID::kMeteor)), m_textures(textures)
{
	Utility::CentreOrigin(m_sprite);
	SetVelocity(-100.f, 0.f); // Move leftward to match horizontal scrolling
}



unsigned int Meteor::GetCategory() const
{
	return static_cast<int>(ReceiverCategories::kEnemyAircraft); // Reuse category for collision handling
}

sf::FloatRect Meteor::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

bool Meteor::IsPowerUp() const
{
	return m_type == MeteorType::kPowerUp;
}

void Meteor::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	Entity::UpdateCurrent(dt, commands); // Ensure inherited movement logic applies
}

void Meteor::MarkForRemoval()
{
	if (IsPowerUp())
	{
		std::unique_ptr<Pickup> pickup = std::make_unique<Pickup>(Pickup::GetRandomPickup(), m_textures);
		pickup->setPosition(getPosition());
		if (GetParent())
		{
			GetParent()->AttachChild(std::move(pickup));
		}
	}
	Entity::MarkForRemoval();
}

void Meteor::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

void Meteor::TakeDamage(int damage)
{
	m_hitpoints -= damage;
	if (m_hitpoints <= 0)
	{
		MarkForRemoval();
	}
}