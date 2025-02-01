#include "ParticleNode.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

ParticleNode::ParticleNode(ParticleType type, const TextureHolder& textures)
	: SceneNode()
	, m_type(type)
	, m_texture(textures.Get(TextureID::kParticle))
	, m_vertex_array(sf::Quads)
{
}

void ParticleNode::AddParticle(sf::Vector2f position)
{
	Particle particle;
	particle.position = position;
	particle.color = sf::Color::White;
	particle.lifetime = 1.0f; // Default lifetime
	m_particles.push_back(particle);
}

ParticleType ParticleNode::GetParticleType() const
{
	return m_type;
}

void ParticleNode::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Reduce particle lifetime
	for (auto& particle : m_particles)
	{
		particle.lifetime -= dt.asSeconds();
	}
	RemoveExpiredParticles();
}

void ParticleNode::RemoveExpiredParticles()
{
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& p) {
			return p.lifetime <= 0;
			}),
		m_particles.end()
	);
}

void ParticleNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = &m_texture;
	target.draw(m_vertex_array, states);
}
