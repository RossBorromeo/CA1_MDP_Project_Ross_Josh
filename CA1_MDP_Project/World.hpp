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

#include <array>

class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& target, FontHolder& fonts, SoundPlayer& sounds, TextureHolder& textures);
	void Update(sf::Time dt);
	void Draw();

	CommandQueue& GetCommandQueue();

	bool HasAlivePlayers() const;
	bool HasPlayerReachedEnd() const;

private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPositions();
	void AdaptPlayerVelocities();

	void SpawnMeteors();
	void AddMeteor();
	sf::FloatRect GetViewBounds() const;
	sf::FloatRect GetBattleFieldBounds() const;

	void DestroyEntitiesOutsideView();
	void HandleCollisions();
	void UpdateSounds();

private:
	struct SpawnPoint
	{
		SpawnPoint(float x, float y, bool is_powerup) : m_x(x), m_y(y), m_is_powerup(is_powerup) {}
		float m_x;
		float m_y;
		bool m_is_powerup;
	};

private:
	sf::RenderTarget& m_target;
	sf::RenderTexture m_scene_texture;
	sf::View m_camera;
	TextureHolder& m_textures;
	FontHolder& m_fonts;
	SoundPlayer& m_sounds;
	SceneNode* m_scenegraph;
	std::array<SceneNode*, static_cast<int>(SceneLayers::kLayerCount)> m_scene_layers;
	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position_p1;
	sf::Vector2f m_spawn_position_p2;
	float m_scrollspeed;
	Aircraft* m_player_one_aircraft;
	Aircraft* m_player_two_aircraft;

	CommandQueue m_command_queue;

	std::vector<SpawnPoint> m_meteor_spawn_points;
};
