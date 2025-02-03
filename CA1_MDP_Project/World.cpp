//Ross - D00241095 | Josh - D00238448
#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include <iostream>

//Ross + Josh was involved in writing in World.cpp (Ross - Random generated enemies + Updated Textures with our own + updated methods. Josh - Added 2nd player aircraft + updated methods) 

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
	, m_player_aircraft1(nullptr)
	, m_player_aircraft2(nullptr)
	, m_rng(m_rd()) //Random number generator
	, m_x_distribution(m_world_bounds.left + 50.f, m_world_bounds.width - 50.f) // Random X positions
	, m_y_distribution(-500.f, -100.f) // Random Y positions
	, m_type_distribution(0, 2) //3 types of enemies
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

	m_player_aircraft1->SetVelocity(0.f, 0.f);
	m_player_aircraft2->SetVelocity(0.f, 0.f);

	DestroyEntitiesOutsideView();
	GuideMissiles();

	// Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayer1Velocity();
	AdaptPlayer2Velocity();

	HandleCollisions1();
	HandleCollisions2();

	m_scenegraph.RemoveWrecks();

	// Randomly spawn new enemies
	GenerateRandomEnemy();

	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayer1Position();
	AdaptPlayer2Position();
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

bool World::HasAlivePlayer1() const
{
	return m_player_aircraft1 != nullptr && !m_player_aircraft1->IsMarkedForRemoval();
}

bool World::HasAlivePlayer2() const
{
	return m_player_aircraft2 != nullptr && !m_player_aircraft2->IsMarkedForRemoval();
}


bool World::HasPlayer1ReachedEnd() const
{
	return !m_world_bounds.contains(m_player_aircraft1->getPosition());

}

bool World::HasPlayer2ReachedEnd() const
{

	return !m_world_bounds.contains(m_player_aircraft2->getPosition());
}

//Ross - Updated and added our own textures for our game.
void World::LoadTextures()
{
	m_textures.Load(TextureID::kBattleShip, "Media/Textures/BattleShip.png");
	m_textures.Load(TextureID::kBattleShip1, "Media/Textures/BattleShip1.png");
	m_textures.Load(TextureID::kMeteor, "Media/Textures/Asteroid.png");
	m_textures.Load(TextureID::kAvenger, "Media/Textures/Meteor.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Space.png");
	m_textures.Load(TextureID::kBullet, "Media/Textures/Laser.png");
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
	m_player_aircraft1 = leader.get();
	m_player_aircraft1->setPosition(m_spawn_position);
	m_player_aircraft1->SetVelocity(40.f, m_scrollspeed);
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(leader));

	std::unique_ptr<Aircraft> leader1(new Aircraft(AircraftType::kBattleShip1, m_textures, m_fonts));
	m_player_aircraft2 = leader1.get();
	m_player_aircraft2->setPosition(m_spawn_position.x + 250.0f, m_spawn_position.y + 1.0f);
	m_player_aircraft2->SetVelocity(40.f, m_scrollspeed);
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(leader1));

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
}

void World::AdaptPlayer1Position()
{
	//keep the player on the screen
	sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	const float border_distance = 40.f;

	sf::Vector2f position1 = m_player_aircraft1->getPosition();
	position1.x = std::max(position1.x, view_bounds.left + border_distance);
	position1.x = std::min(position1.x, view_bounds.left + view_bounds.width - border_distance);
	position1.y = std::max(position1.y, view_bounds.top + border_distance);
	position1.y = std::min(position1.y, view_bounds.top + view_bounds.height - border_distance);
	m_player_aircraft1->setPosition(position1);


}
void World::AdaptPlayer2Position()
{
	sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	const float border_distance = 40.f;

	sf::Vector2f position2 = m_player_aircraft2->getPosition();
	position2.x = std::max(position2.x, view_bounds.left + border_distance);
	position2.x = std::min(position2.x, view_bounds.left + view_bounds.width - border_distance);
	position2.y = std::max(position2.y, view_bounds.top + border_distance);
	position2.y = std::min(position2.y, view_bounds.top + view_bounds.height - border_distance);
	m_player_aircraft2->setPosition(position2);
}


void World::AdaptPlayer1Velocity()
{
	sf::Vector2f velocity1 = m_player_aircraft1->GetVelocity();

	//If they are moving diagonally divide by sqrt 2
	if (velocity1.x != 0.f && velocity1.y != 0.f)
	{
		m_player_aircraft1->SetVelocity(velocity1 / std::sqrt(2.f));
	}
	//Add scrolling velocity
	m_player_aircraft1->Accelerate(0.f, m_scrollspeed);

	sf::Vector2f velocity2 = m_player_aircraft2->GetVelocity();


}

void World::AdaptPlayer2Velocity()
{
	sf::Vector2f velocity2 = m_player_aircraft2->GetVelocity();
	//If they are moving diagonally divide by sqrt 2
	if (velocity2.x != 0.f && velocity2.y != 0.f)
	{
		m_player_aircraft2->SetVelocity(velocity2 / std::sqrt(2.f));
	}
	//Add scrolling velocity
	m_player_aircraft2->Accelerate(0.f, m_scrollspeed);
}

//Ross - Added code that allowed the meteors to spawn in randomly, creating the main gameplay aspect of our game.
void World::GenerateRandomEnemy()
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

		//Spawn Above player
		float min_y = m_camera.getCenter().y - screen_height / 2.f - 100.f; 
		float max_y = m_camera.getCenter().y - screen_height / 2.f - 50.f; 

		std::uniform_real_distribution<float> x_distribution(min_x, max_x);
		std::uniform_real_distribution<float> y_distribution(min_y, max_y);

		std::vector<AircraftType> enemy_types = { AircraftType::kMeteor };
		std::uniform_int_distribution<int> type_distribution(0, enemy_types.size() - 1);
		AircraftType type = enemy_types[type_distribution(m_rng)];

		float x = x_distribution(m_rng);
		float y = y_distribution(m_rng);

		// Create enemy
		std::unique_ptr<Aircraft> enemy = std::make_unique<Aircraft>(type, m_textures, m_fonts);
		enemy->setPosition(x, y);
		enemy->setRotation(180.f); 

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


void World::HandleCollisions1()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for (SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft1, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);



			// Reduce damage 
			player.Damage(5);

			// Move Player 1 back to the bottom of the screen
			player.setPosition(m_spawn_position);



			enemy.Destroy();
		}

		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft1, ReceiverCategories::kEnemyProjectile) ||
			MatchesCategories(pair, ReceiverCategories::kEnemyAircraft, ReceiverCategories::kAlliedProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			// Normal projectile damage
			aircraft.Damage(projectile.GetDamage());
			projectile.Destroy();
		}
	}
}


void World::HandleCollisions2()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for (SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft2, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);


			// Reduce damage 
			player.Damage(5);

			// Move Player 2 back to the bottom of the screen
			player.setPosition(m_spawn_position.x + 250.0f, m_spawn_position.y + 1.0f);
			enemy.Destroy();
		}

		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft2, ReceiverCategories::kEnemyProjectile) ||
			MatchesCategories(pair, ReceiverCategories::kEnemyAircraft, ReceiverCategories::kAlliedProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			// Normal projectile damage
			aircraft.Damage(projectile.GetDamage());
			projectile.Destroy();
		}
	}
}




void World::UpdateSounds()
{
	// Set listener's position to player position
	m_sounds.SetListenerPosition(m_player_aircraft1->GetWorldPosition());

	m_sounds.SetListenerPosition(m_player_aircraft2->GetWorldPosition());

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}