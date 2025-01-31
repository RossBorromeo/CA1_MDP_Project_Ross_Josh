#pragma once
#include "SceneNode.hpp"
#include <SFML/System/Vector2.hpp>

class Entity : public SceneNode
{
public:
	void SetVelocity(sf::Vector2f velocity);
	void SetVelocity(float vx, float vy);
	sf::Vector2f GetVelocity() const;

	void Damage(int points);
	void Destroy();
	bool IsDestroyed() const;

protected:
	explicit Entity(int hitpoints);
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;

private:
	sf::Vector2f m_velocity;
	int m_hitpoints;
};
