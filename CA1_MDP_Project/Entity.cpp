#include "Entity.hpp"

Entity::Entity(int hitpoints)
	: SceneNode(), m_velocity(0.f, 0.f), m_hitpoints(hitpoints)
{
}

void Entity::SetVelocity(sf::Vector2f velocity)
{
	m_velocity = velocity;
}

void Entity::SetVelocity(float vx, float vy)
{
	m_velocity.x = vx;
	m_velocity.y = vy;
}

sf::Vector2f Entity::GetVelocity() const
{
	return m_velocity;
}

void Entity::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	move(m_velocity * dt.asSeconds()); // Apply movement
}

void Entity::Damage(int points)
{
	m_hitpoints -= points;
	if (m_hitpoints <= 0)
	{
		Destroy();
	}
}

void Entity::Destroy()
{
	MarkForRemoval(); // Updated to use a proper SceneNode removal function
}

bool Entity::IsDestroyed() const
{
	return m_hitpoints <= 0;
}
