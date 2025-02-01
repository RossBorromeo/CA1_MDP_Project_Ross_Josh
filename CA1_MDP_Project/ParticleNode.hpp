#pragma once
#include "SceneNode.hpp"
#include "ParticleType.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <deque>

struct Particle
{
	sf::Vector2f position;
	sf::Color color;
	float lifetime;
};

class ParticleNode : public SceneNode
{
public:
	explicit ParticleNode(ParticleType type, const TextureHolder& textures);
	void AddParticle(sf::Vector2f position);
	ParticleType GetParticleType() const;

private:
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void RemoveExpiredParticles();

private:
	std::deque<Particle> m_particles;
	sf::VertexArray m_vertex_array;
	ParticleType m_type;
	const sf::Texture& m_texture;
};
