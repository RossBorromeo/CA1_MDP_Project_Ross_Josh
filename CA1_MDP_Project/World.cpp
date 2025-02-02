#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include <iostream>

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, 3000.f)
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.height - m_camera.getSize().y / 2.f)
	, m_scrollspeed(-50.f)
	, m_player_aircraft(nullptr)
	, m_rng(m_rd()) // Initialize random number generator
	, m_x_distribution(m_world_bounds.left + 50.f, m_world_bounds.width - 50.f) // Random X positions
	, m_y_distribution(-500.f, -100.f) // Random Y positions (above screen)
	, m_type_distribution(0, 2) // 3 types of enemies
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
}

void World::Update(sf::Time dt)
{
	// Scroll the world
	m_camera.move(0, m_scrollspeed * dt.asSeconds());

	m_player_aircraft->SetVelocity(0.f, 0.f);

	DestroyEntitiesOutsideView();
	GuideMissiles();

	// Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	HandleCollisions();

	m_scenegraph.RemoveWrecks();

	// Randomly spawn new enemies
	GenerateRandomEnemy();

	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayerPosition();
}

void World::Draw()
{
	if (PostEffect::IsSupported())
	{
		m_scene_texture.clear();
		m_scene_texture.setView(m_camera);
		m_scene_texture.draw(m_scenegraph);
		m_scene_texture.display();
		m_bloom_effect.Apply(m_scene_texture, m_target);
	}
	else
	{
		m_target.setView(m_camera);
		m_target.draw(m_scenegraph);
	}
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

bool World::HasAlivePlayer() const
{
	return !m_player_aircraft->IsMarkedForRemoval();
}

bool World::HasPlayerReachedEnd() const
{
	return !m_world_bounds.contains(m_player_aircraft->getPosition());
}

void World::LoadTextures()
{
	m_textures.Load(TextureID::kBattleShip, "Media/Textures/BattleShip.png");
	m_textures.Load(TextureID::kMeteor, "Media/Textures/Asteroid.png");
	m_textures.Load(TextureID::kAvenger, "Media/Textures/Meteor.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Space.png");
	m_textures.Load(TextureID::kBullet, "Media/Textures/Bullet.png");
	m_textures.Load(TextureID::kMissile, "Media/Textures/Missile.png");

	m_textures.Load(TextureID::kHealthRefill, "Media/Textures/HealthRefill.png");
	m_textures.Load(TextureID::kMissileRefill, "Media/Textures/MissileRefill.png");
	m_textures.Load(TextureID::kFireSpread, "Media/Textures/FireSpread.png");
	m_textures.Load(TextureID::kFireRate, "Media/Textures/FireRate.png");
	m_textures.Load(TextureID::kFinishLine, "Media/Textures/FinishLine.png");

	m_textures.Load(TextureID::kEntities, "Media/Textures/Entities.png");
	m_textures.Load(TextureID::kSpace, "Media/Textures/Space.png");
	m_textures.Load(TextureID::kExplosion, "Media/Textures/Explosion.png");
	m_textures.Load(TextureID::kParticle, "Media/Textures/Particle.png");


}

void World::BuildScene()
{
	//Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(SceneLayers::kLayerCount); ++i)
	{
		ReceiverCategories category = (i == static_cast<int>(SceneLayers::kLowerAir)) ? ReceiverCategories::kScene : ReceiverCategories::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(TextureID::kSpace);
	sf::IntRect textureRect(m_world_bounds);
	texture.setRepeated(true);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//Add the finish line
	sf::Texture& finish_texture = m_textures.Get(TextureID::kFinishLine);
	std::unique_ptr<SpriteNode> finish_sprite(new SpriteNode(finish_texture));
	finish_sprite->setPosition(0.f, -76.f);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(finish_sprite));

	//Add the player's aircraft
	std::unique_ptr<Aircraft> leader(new Aircraft(AircraftType::kBattleShip, m_textures, m_fonts));
	m_player_aircraft = leader.get();
	m_player_aircraft->setPosition(m_spawn_position);
	m_player_aircraft->SetVelocity(40.f, m_scrollspeed);
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(leader));

	//Add the particle nodes to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(smokeNode));

	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	/*std::unique_ptr<Aircraft> left_escort(new Aircraft(AircraftType::kMeteor, m_textures, m_fonts));
	left_escort->setPosition(-80.f, 50.f);
	m_player_aircraft->AttachChild(std::move(left_escort));

	std::unique_ptr<Aircraft> right_escort(new Aircraft(AircraftType::kMeteor, m_textures, m_fonts));
	right_escort->setPosition(80.f, 50.f);
	m_player_aircraft->AttachChild(std::move(right_escort));*/
}

void World::AdaptPlayerPosition()
{
	//keep the player on the screen
	sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	const float border_distance = 40.f;

	sf::Vector2f position = m_player_aircraft->getPosition();
	position.x = std::max(position.x, view_bounds.left + border_distance);
	position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
	position.y = std::max(position.y, view_bounds.top + border_distance);
	position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance);
	m_player_aircraft->setPosition(position);
}

void World::AdaptPlayerVelocity()
{
	sf::Vector2f velocity = m_player_aircraft->GetVelocity();

	//If they are moving diagonally divide by sqrt 2
	if (velocity.x != 0.f && velocity.y != 0.f)
	{
		m_player_aircraft->SetVelocity(velocity / std::sqrt(2.f));
	}
	//Add scrolling velocity
	m_player_aircraft->Accelerate(0.f, m_scrollspeed);
}

void World::GenerateRandomEnemy()
{
	static sf::Clock spawn_timer;
	sf::Time spawn_interval = sf::seconds(0.5f); // Spawn every 2 seconds

	if (spawn_timer.getElapsedTime() >= spawn_interval)
	{
		float screen_width = m_target.getSize().x;
		float screen_height = m_target.getSize().y;

		// Ensure enemies spawn inside the screen width
		float min_x = 50.f;
		float max_x = screen_width - 50.f;

		// Instead of spawning deep below, spawn slightly above the player
		float min_y = m_camera.getCenter().y - screen_height / 2.f - 100.f; // Above screen
		float max_y = m_camera.getCenter().y - screen_height / 2.f - 50.f; // Not too far

		std::uniform_real_distribution<float> x_distribution(min_x, max_x);
		std::uniform_real_distribution<float> y_distribution(min_y, max_y);

		std::vector<AircraftType> enemy_types = { AircraftType::kMeteor};
		std::uniform_int_distribution<int> type_distribution(0, enemy_types.size() - 1);
		AircraftType type = enemy_types[type_distribution(m_rng)];

		float x = x_distribution(m_rng);
		float y = y_distribution(m_rng);

		std::cout << "Enemy spawned at: " << x << ", " << y << std::endl; // Debug print

		// Create enemy
		std::unique_ptr<Aircraft> enemy = std::make_unique<Aircraft>(type, m_textures, m_fonts);
		enemy->setPosition(x, y);
		enemy->setRotation(180.f); // Face downward

		// Add to scene graph
		m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));

		spawn_timer.restart();
	}
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattleFieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;

}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kEnemyAircraft) | static_cast<int>(ReceiverCategories::kProjectile);
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
		{
			// Give enemies some buffer before removing them
			sf::FloatRect battlefield_bounds = GetBattleFieldBounds();
			battlefield_bounds.top -= 100.f;  // Allow some buffer before removing enemies
			battlefield_bounds.height += 200.f;

			if (!battlefield_bounds.intersects(e.GetBoundingRect()))
			{
				e.Destroy();
			}
		});
	m_command_queue.Push(command);
}

void World::GuideMissiles()
{
	//Target the closest enemy in the world
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(ReceiverCategories::kEnemyAircraft);
	enemyCollector.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
		{
			if (!enemy.IsDestroyed())
			{
				m_active_enemies.emplace_back(&enemy);
			}
		});

	Command missileGuider;
	missileGuider.category = static_cast<int>(ReceiverCategories::kAlliedProjectile);
	missileGuider.action = DerivedAction<Projectile>([this](Projectile& missile, sf::Time dt)
		{
			if (!missile.IsGuided())
			{
				return;
			}

			float min_distance = std::numeric_limits<float>::max();
			Aircraft* closest_enemy = nullptr;

			for (Aircraft* enemy : m_active_enemies)
			{
				float enemy_distance = Distance(missile, *enemy);
				if (enemy_distance < min_distance)
				{
					closest_enemy = enemy;
					min_distance = enemy_distance;
				}
			}

			if (closest_enemy)
			{
				missile.GuideTowards(closest_enemy->GetWorldPosition());
			}
		});

	m_command_queue.Push(enemyCollector);
	m_command_queue.Push(missileGuider);
	m_active_enemies.clear();
}

bool MatchesCategories(SceneNode::Pair& colliders, ReceiverCategories type1, ReceiverCategories type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();

	if (static_cast<int>(type1) & category1 && static_cast<int>(type2) & category2)
	{
		return true;
	}
	else if (static_cast<int>(type1) & category2 && static_cast<int>(type2) & category1)
	{
		std::swap(colliders.first, colliders.second);
	}
	else
	{
		return false;
	}
}


void World::HandleCollisions()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for (SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);
			//Collision response
			player.Damage(enemy.GetHitPoints());
			enemy.Destroy();
		}

		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kPickup))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);
			//Collision response
			pickup.Apply(player);
			pickup.Destroy();
			player.PlayLocalSound(m_command_queue, SoundEffect::kCollectPickup);
		}
		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyProjectile) || MatchesCategories(pair, ReceiverCategories::kEnemyAircraft, ReceiverCategories::kAlliedProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			//Collision response
			aircraft.Damage(projectile.GetDamage());
			projectile.Destroy();
		}
	}
}

void World::UpdateSounds()
{
	// Set listener's position to player position
	m_sounds.SetListenerPosition(m_player_aircraft->GetWorldPosition());

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}
