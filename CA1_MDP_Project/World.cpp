#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include "Meteor.hpp"
#include "Aircraft.hpp"
#include "Utility.hpp"
#include "Command.hpp"
#include "SceneLayers.hpp"
#include "ResourceHolder.hpp"
#include "ReceiverCategories.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, TextureHolder& textures)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures(textures)
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph(std::make_unique<SceneNode>())
	, m_world_bounds(0.f, 0.f, 5000.f, m_camera.getSize().y)
	, m_spawn_position_p1(100.f, m_camera.getSize().y - 100.f)
	, m_spawn_position_p2(100.f, 100.f)
	, m_scrollspeed(50.f)
	, m_player_one_aircraft(nullptr)
	, m_player_two_aircraft(nullptr)
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position_p1.x, m_camera.getSize().y / 2.f);
}

void World::Update(sf::Time dt)
{
	// Scroll the world horizontally
	m_camera.move(m_scrollspeed * dt.asSeconds(), 0);

	m_player_one_aircraft->SetVelocity(0.f, 0.f);
	m_player_two_aircraft->SetVelocity(0.f, 0.f);

	DestroyEntitiesOutsideView();
	HandleCollisions();

	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph->OnCommand(m_command_queue.Pop(), dt);
	}

	AdaptPlayerVelocities();
	m_scenegraph->RemoveWrecks();
	SpawnMeteors();
	m_scenegraph->Update(dt, m_command_queue);
	AdaptPlayerPositions();
}

void World::Draw()
{
	m_target.setView(m_camera);
	m_target.draw(*m_scenegraph);
}

void World::HandleCollisions()
{
	for (auto& projectile : m_scenegraph->GetChildren())
	{
		if (Projectile* proj = dynamic_cast<Projectile*>(projectile.get()))
		{
			for (auto& entity : m_scenegraph->GetChildren())
			{
				if (Aircraft* aircraft = dynamic_cast<Aircraft*>(entity.get()))
				{
					if (proj->GetBoundingRect().intersects(aircraft->GetBoundingRect()))
					{
						aircraft->TakeDamage(1);
						proj->MarkForRemoval();
					}
				}
				else if (Meteor* meteor = dynamic_cast<Meteor*>(entity.get()))
				{
					if (proj->GetBoundingRect().intersects(meteor->GetBoundingRect()))
					{
						meteor->TakeDamage(1);
						proj->MarkForRemoval();
					}
				}
			}
		}
	}
}
