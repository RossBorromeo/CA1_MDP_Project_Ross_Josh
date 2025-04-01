//D00238448:Joshua Thompson
//D00241095:Ross Borromeo

#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include "SpriteNode.hpp"
#include <iostream>


World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, 3000.f)
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.height - m_camera.getSize().y / 2.f)
	, m_scrollspeed(-3.f)
	, m_player_aircraft()
	, m_rng(m_rd()) // Initialize random number generator
	, m_x_distribution(m_world_bounds.left + 50.f, m_world_bounds.width - 50.f) // Random X positions
	, m_y_distribution(-500.f, -100.f) // Random Y positions (above screen)
	, m_type_distribution(0, 2) // 3 types of enemies
	, m_enemy_spawn_points()
	, m_active_enemies()
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
	, m_spawn_timer()
	, m_spawn_interval()
{

	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);

}

void World::SetWorldScrollCompensation(float compensation)
{
	m_scrollspeed_compensation = compensation;
}

void World::Update(sf::Time dt)
{
	UpdateBackground(dt.asSeconds());
	m_spawn_timer += dt;
	if (m_spawn_timer >= m_spawn_interval)
	{
		GenerateRandomEnemy();
		m_spawn_timer = sf::Time::Zero;
	}

	m_camera.move(0, m_scrollspeed * dt.asSeconds());

	if (m_camera.getCenter().y <= 0.f)
	{
		m_camera.setCenter(m_camera.getCenter().x, m_world_bounds.height);
	}
	
	// 2. THEN zero velocity AFTER command application
	for (Aircraft* aircraft : m_player_aircraft)
		aircraft->SetVelocity(0.f, 0.f);


	// 1. Process commands FIRST
	while (!m_command_queue.IsEmpty())
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);



	DestroyEntitiesOutsideView();
	GuideMissiles();
	AdaptPlayerVelocity();
	HandleCollisions();

	auto first_to_remove = std::remove_if(
		m_player_aircraft.begin(), m_player_aircraft.end(),
		std::mem_fn(&Aircraft::IsMarkedForRemoval));
	m_player_aircraft.erase(first_to_remove, m_player_aircraft.end());

	m_scenegraph.RemoveWrecks();
	SpawnEnemies();
	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayerPosition();
	UpdateSounds();
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



Aircraft* World::GetAircraft(int identifier) const
{
	for (Aircraft* a : m_player_aircraft)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

void World::RemoveAircraft(int identifier)
{
	Aircraft* aircraft = GetAircraft(identifier);
	if (aircraft)
	{
		aircraft->Destroy();

	}
}

Aircraft* World::AddAircraft(int identifier)
{
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftType::kBattleShip, m_textures, m_fonts));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);
	player->SetRespawnPosition(player->getPosition());
	m_player_aircraft.emplace_back(player.get());
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player));
	return m_player_aircraft.back();
}




bool World::PollGameAction(GameActions::Action& out)
{
	return m_network_node->PollGameAction(out);
}

void World::SetCurrentBattleFieldPosition(float lineY)
{
	m_camera.setCenter(m_camera.getCenter().x, lineY - m_camera.getSize().y / 2);
	m_spawn_position.y = m_world_bounds.height;
}

void World::SetWorldHeight(float height)
{
	m_world_bounds.height = height;
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

bool World::HasAlivePlayer() const
{
	return !m_player_aircraft.empty();
}


//bool World::HasPlayerReachedEnd() const
//{
//	if (Aircraft* aircraft = GetAircraft(1))
//	{
//		return !m_world_bounds.contains(aircraft->getPosition());
//	}
//	return false;
//}

//void World::CreatePickup(sf::Vector2f position, PickupType type)
//{
//	std::unique_ptr<Pickup> pickup(new Pickup(type, m_textures));
//	pickup->setPosition(position);
//	pickup->SetVelocity(0.f, 1.f);
//	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(pickup));
////}

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
	// Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(SceneLayers::kLayerCount); ++i)
	{
		ReceiverCategories category = (i == static_cast<int>(SceneLayers::kLowerAir)) ? ReceiverCategories::kScene : ReceiverCategories::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	// Prepare the background
	sf::Texture& texture = m_textures.Get(TextureID::kSpace);
	sf::IntRect textureRect(m_world_bounds);
	texture.setRepeated(true);

	// Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	// Add particle nodes (for visual effects)
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(smokeNode));

	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode());
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}

	// Add enemies
	AddEnemies();
}



void World::AdaptPlayerPosition() 

{
	//Keep the players on the screen 
	sf::FloatRect view_bounds = GetViewBounds();
	const float border_distance = 40.f;

	for (Aircraft* aircraft : m_player_aircraft)
	{
		sf::Vector2f position = aircraft->getPosition();
		position.x = std::max(position.x, view_bounds.left + border_distance);
		position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
		position.y = std::max(position.y, view_bounds.top + border_distance);
		position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance);
		aircraft->setPosition(position);
	}


}

void World::AdaptPlayerVelocity() //changed by Josh added in secondary player functionality
{
	for (Aircraft* aircraft : m_player_aircraft)
	{
		sf::Vector2f velocity = aircraft->GetVelocity();

		//If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
		{
			aircraft->SetVelocity(velocity / std::sqrt(2.f));
		}

		//Add scrolling velocity
		aircraft->Accelerate(0.f, m_scrollspeed);
	}

}

//Ross did this
void World::GenerateRandomEnemy()
{
	

	static sf::Clock meteor_timer;
	static sf::Clock avenger_timer;

	sf::Time meteor_spawn_interval = sf::seconds(0.45f);  // Meteor spawns every 0.45s
	sf::Time avenger_spawn_interval = sf::seconds(1.5f);  // Avenger spawns every 1.5s

	float screen_width = m_target.getSize().x;
	float screen_height = m_target.getSize().y;

	float min_x = 50.f;
	float max_x = screen_width - 50.f;

	float min_y = m_camera.getCenter().y - screen_height / 2.f - 100.f; // Slightly above screen
	float max_y = m_camera.getCenter().y - screen_height / 2.f - 50.f;

	std::uniform_real_distribution<float> x_distribution(min_x, max_x);
	std::uniform_real_distribution<float> y_distribution(min_y, max_y);

	// Spawn KMeteor at regular intervals
	if (meteor_timer.getElapsedTime() >= meteor_spawn_interval)
	{
		float x = x_distribution(m_rng);
		float y = y_distribution(m_rng);

		std::unique_ptr<Aircraft> meteor = std::make_unique<Aircraft>(AircraftType::kMeteor, m_textures, m_fonts);
		meteor->setPosition(x, y);
		meteor->setRotation(180.f); 

		m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(meteor));
		meteor_timer.restart();
	}

	// Spawn KAvenger at a slower rate
	if (avenger_timer.getElapsedTime() >= avenger_spawn_interval)
	{
		float x = x_distribution(m_rng);
		float y = y_distribution(m_rng);

		std::unique_ptr<Aircraft> avenger = std::make_unique<Aircraft>(AircraftType::kAvenger, m_textures, m_fonts);
		avenger->setPosition(x, y);
		avenger->setRotation(180.f); 

		m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(avenger));
		avenger_timer.restart();
	}
}


sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;

}

void World::SpawnEnemies()
{
	if (m_networked_world)
	{
		return;
	}

	while (!m_enemy_spawn_points.empty() && m_enemy_spawn_points.back().m_y > GetBattlefieldBounds().top)
	{
		SpawnPoint spawn = m_enemy_spawn_points.back();
		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.m_type, m_textures, m_fonts));
		enemy->setPosition(spawn.m_x, spawn.m_y);
		enemy->setRotation(180.f);

		m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));
		m_enemy_spawn_points.pop_back();
	}
}

void World::AddEnemy(AircraftType type, float relx, float rely)
{
	SpawnPoint spawn(type, m_spawn_position.x + relx, m_spawn_position.y - rely);
	m_enemy_spawn_points.emplace_back(spawn);
}

void World::AddEnemies()
{
	if (m_networked_world)
	{
		return;
	}

	//Sort according to y value so that lower enemies are checked first
	SortEnemies();
}

void World::SortEnemies()
{
	//Sort all enemies according to their y-value, such that lower enemies are checked first for spawning
	std::sort(m_enemy_spawn_points.begin(), m_enemy_spawn_points.end(), [](SpawnPoint lhs, SpawnPoint rhs)
		{
			return lhs.m_y < rhs.m_y;
		});
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kEnemyAircraft) | static_cast<int>(ReceiverCategories::kProjectile);
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
		{
			// Give enemies some buffer before removing them
			sf::FloatRect battlefield_bounds = GetBattlefieldBounds();
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
		// Player <-> Enemy Aircraft
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);


			//Josh added Invincibility
			if (!player.IsInvincible())
			{
				// Deal damage
				if (enemy.GetType() == AircraftType::kAvenger)
					player.Damage(30);
				else
					player.Damage(10);

				//  Destroy the player if HP now 0 or less
				if (player.GetHitPoints() <= 0)
				{
					player.Destroy(); //  essential
				}
				else
				{
					player.StartInvincibility();
					player.setPosition(player.GetRespawnPosition());
				}
			}

			enemy.Destroy();
		}

		// Player hit by enemy projectile or enemy hit by player projectile
		else if (
			MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyProjectile) ||
			MatchesCategories(pair, ReceiverCategories::kEnemyAircraft, ReceiverCategories::kAlliedProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			if (!aircraft.IsInvincible())
			{
				aircraft.Damage(projectile.GetDamage());

				if (aircraft.GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft))
					aircraft.StartInvincibility();
			}

			projectile.Destroy();
		}
	}
}

void World::UpdateBackground(float deltaTime)
{
	const float scrollSpeed = 3.0f; // Adjust based on your game speed

	for (auto& background : m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->GetChildren())
	{
		// Move the background downward
		background->move(0, scrollSpeed * deltaTime);

		// Check if the background has moved completely out of view
		if (background->getPosition().y >= m_world_bounds.height)
		{
			// Reset its position to the top
			background->move(0, -2 * m_world_bounds.height);
		}
	}
}


void World::UpdateSounds()
{
	sf::Vector2f listener_position;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (m_player_aircraft.empty())
	{
		listener_position = m_camera.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for (Aircraft* aircraft : m_player_aircraft)
		{
			listener_position += aircraft->GetWorldPosition();
		}

		listener_position /= static_cast<float>(m_player_aircraft.size());
	}

	// Set listener's position
	m_sounds.SetListenerPosition(listener_position);

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}