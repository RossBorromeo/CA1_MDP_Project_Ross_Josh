#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
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
	, m_scrollspeed(-50.f)
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
	// Scroll the world
	m_camera.move(0, m_scrollspeed * dt.asSeconds());

	for (Aircraft* a : m_player_aircraft)
	{
		a->SetVelocity(0.f, 0.f);
	}


	DestroyEntitiesOutsideView();
	GuideMissiles();

	// Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();


	HandleCollisions();

	auto first_to_remove = std::remove_if(m_player_aircraft.begin(), m_player_aircraft.end(), std::mem_fn(&Aircraft::IsMarkedForRemoval));
	m_player_aircraft.erase(first_to_remove, m_player_aircraft.end());
	m_scenegraph.RemoveWrecks();

	// Randomly spawn new enemies
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

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

bool World::HasAlivePlayer() const
{
	return !m_player_aircraft.empty();
}




bool World::HasPlayerReachedEnd() const
{
	if (Aircraft* aircraft = GetAircraft(1))
	{
		return !m_world_bounds.contains(aircraft->getPosition());
	}
	return false;
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
		m_player_aircraft.erase(std::find(m_player_aircraft.begin(), m_player_aircraft.end(), aircraft));
	}
}

Aircraft* World::AddAircraft(int identifier)
{
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftType::kBattleShip, m_textures, m_fonts));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);

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
	m_finish_sprite = finish_sprite.get();
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(finish_sprite));

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
	m_player_aircraft1->AttachChild(std::move(left_escort));

	std::unique_ptr<Aircraft> right_escort(new Aircraft(AircraftType::kMeteor, m_textures, m_fonts));
	right_escort->setPosition(80.f, 50.f);
	m_player_aircraft1->AttachChild(std::move(right_escort));*/

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode());
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}

	AddEnemies();
}

void World::AdaptPlayerPosition() //changed by Josh added in secondary player functionality
{
	//Keep the players on the sceen 
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



/*void World::GenerateRandomEnemy()
{
	static sf::Clock spawn_timer;
	sf::Time spawn_interval = sf::seconds(0.45f); // Spawn every .45 seconds

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

		std::vector<AircraftType> enemy_types = { AircraftType::kMeteor };
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
}*/

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
	//Spawn an enemy when it is relevant i.e when it is in the Battlefieldboudns
	while (!m_enemy_spawn_points.empty() && m_enemy_spawn_points.back().m_y > GetBattlefieldBounds().top)
	{
		SpawnPoint spawn = m_enemy_spawn_points.back();
		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.m_type, m_textures, m_fonts));
		enemy->setPosition(spawn.m_x, spawn.m_y);
		enemy->setRotation(180.f);

		//If the game is networked the server is responsible for spawning pickups


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
	//Add all emenies
	AddEnemy(AircraftType::kMeteor, 0.f, 500.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 1000.f);
	AddEnemy(AircraftType::kMeteor, +100.f, 1150.f);
	AddEnemy(AircraftType::kMeteor, -100.f, 1150.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1710.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1700.f);
	AddEnemy(AircraftType::kAvenger, 30.f, 1850.f);
	AddEnemy(AircraftType::kMeteor, 300.f, 2200.f);
	AddEnemy(AircraftType::kMeteor, -300.f, 2200.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 2200.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 2500.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 3000.f);
	AddEnemy(AircraftType::kMeteor, 250.f, 3250.f);
	AddEnemy(AircraftType::kMeteor, -250.f, 3250.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3500.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3700.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 3800.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 4000.f);
	AddEnemy(AircraftType::kAvenger, -200.f, 4200.f);
	AddEnemy(AircraftType::kMeteor, 200.f, 4200.f);
	AddEnemy(AircraftType::kMeteor, 0.f, 4400.f);

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
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);
			//Collision response
			player.Damage(enemy.GetHitPoints());
			enemy.Destroy();
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