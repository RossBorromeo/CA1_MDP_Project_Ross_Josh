//D00238448:Joshua Thompson
//d00241095:Ross Borromeo
#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceIdentifiers.hpp"
#include "ResourceHolder.hpp"
#include "SceneNode.hpp"
#include "SceneLayers.hpp"
#include "Aircraft.hpp"
#include "TextureID.hpp"
#include "SpriteNode.hpp"
#include "CommandQueue.hpp"
#include "BloomEffect.hpp"
#include "SoundPlayer.hpp"

#include <SFML/Network/Packet.hpp>

#include <array>
#include "PickupType.hpp"
#include "NetworkNode.hpp"




class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& target, FontHolder& font, SoundPlayer& sounds, bool networked = false);
	void Update(sf::Time dt);
	void Draw();

	sf::FloatRect GetViewBounds() const;
	CommandQueue& GetCommandQueue();

	Aircraft* AddAircraft(int identifier);
	void RemoveAircraft(int identifier);
	void SetCurrentBattleFieldPosition(float line_y);
	void SetWorldHeight(float height);

	void AddEnemy(AircraftType type, float relx, float rely);
	void SpawnEnemies();
	void SortEnemies();

	bool HasAlivePlayer() const;
	bool HasPlayerReachedEnd() const;

	void SetWorldScrollCompensation(float compensation);
	Aircraft* GetAircraft(int identifier) const;
	sf::FloatRect GetBattlefieldBounds() const;
	bool PollGameAction(GameActions::Action& out);

	

private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPosition();
	void AdaptPlayerVelocity();
	void GenerateRandomEnemy();
	void AddEnemies();

	void DestroyEntitiesOutsideView();
	void GuideMissiles();

	void HandleCollisions();
	void UpdateSounds();

private:
	struct SpawnPoint
	{
		SpawnPoint(AircraftType type, float x, float y) : m_type(type), m_x(x), m_y(y) {}
		AircraftType m_type;
		float m_x;
		float m_y;
	};

private:
	sf::RenderTarget& m_target;
	sf::RenderTexture m_scene_texture;
	sf::View m_camera;
	TextureHolder m_textures;
	FontHolder& m_fonts;
	SoundPlayer& m_sounds;
	SceneNode m_scenegraph;
	std::array<SceneNode*, NumSceneLayers> m_scene_layers;
	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position;
	float m_scrollspeed;
	float m_scrollspeed_compensation;
	sf::Time m_spawn_timer;
	sf::Time m_spawn_interval;

	std::vector<Aircraft*> m_player_aircraft;

	CommandQueue m_command_queue;

	std::vector<SpawnPoint> m_enemy_spawn_points;
	std::vector<Aircraft*> m_active_enemies;

	BloomEffect m_bloom_effect;
	bool m_networked_world;
	NetworkNode* m_network_node;
	SpriteNode* m_finish_sprite;

	// Random number generation
	std::random_device m_rd;
	std::mt19937 m_rng;
	std::uniform_real_distribution<float> m_x_distribution;
	std::uniform_real_distribution<float> m_y_distribution;
	std::uniform_int_distribution<int> m_type_distribution;
};
