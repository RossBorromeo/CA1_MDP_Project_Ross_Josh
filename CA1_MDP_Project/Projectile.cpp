#include "Projectile.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "ReceiverCategories.hpp"

Projectile::Projectile(ProjectileType type, const TextureHolder& textures)
	: Entity(1) // Default HP (for potential destruction on collision)
	, m_type(type)
	, m_sprite(textures.Get(type == ProjectileType::kMissile ? TextureID::kMissile : TextureID::kBullet))
{
	Utility::CentreOrigin(m_sprite);
}

void Projectile::GuideTowards(sf::Vector2f position)
{
	m_target_direction = Utility::UnitVector(position - GetWorldPosition());
}

unsigned int Projectile::GetCategory() const
{
	if (m_type == ProjectileType::kMissile)
		return static_cast<int>(ReceiverCategories::kPlayerProjectile);
	else
		return static_cast<int>(ReceiverCategories::kEnemyProjectile);
}

sf::FloatRect Projectile::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

float Projectile::GetDamage() const
{
	return (m_type == ProjectileType::kMissile) ? 50.f : 10.f;
}

void Projectile::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (!m_target_direction.x == 0.f || !m_target_direction.y == 0.f)
	{
		SetVelocity(m_target_direction * 300.f);
	}
	SetVelocity(GetVelocity() * dt.asSeconds());
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}
