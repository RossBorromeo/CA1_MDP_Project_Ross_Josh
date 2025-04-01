//Ross - D00241095 | Josh - D00238448
#pragma once

const unsigned short SERVER_PORT = 50001; // Use dynamic port range
const float TICK_RATE = 10.f;
const float FRAME_RATE = 60.f;

const unsigned short MAX_CONNECTIONS = 15;

const float WINDOW_WIDTH = 1920.f;
const float WINDOW_HEIGHT = 1080.f;

namespace Server
{
	enum class PacketType
	{
		kBroadcastMessage,       // std::string shown on all clients' screens
		kInitialState,           // World height, scroll offset, aircraft count + per-aircraft info
		kPlayerEvent,            // Aircraft ID + action
		kPlayerRealtimeChange,   // Aircraft ID + action + bool
		kPlayerConnect,          // Like SpawnSelf but for other players
		kPlayerDisconnect,       // Aircraft ID
		kSpawnEnemy,             // AircraftType ID + position (x, y)
		kSpawnSelf,              // Aircraft ID + position (x, y)
		kUpdateClientState,      // Aircraft states + positions
		kMissionSuccess,         // No args � indicates game win
		kWaitingNotice,          // Server waiting for all players to be ready
		kGameReady,              // All players ready  start game
	};
}

namespace Client
{
	enum class PacketType
	{
		kPlayerEvent,            // Aircraft ID + action
		kPlayerRealtimeChange,   // Aircraft ID + action + bool
		kStateUpdate,            // Aircraft data (ID, pos, stats)
		kGameEvent,              // Used for explosions
		kQuit,                   // Disconnect request
		kReadyNotice             // Toggle readiness state (ENTER press)
	};
}

namespace GameActions
{
	enum Type
	{
		kEnemyExplode
	};

	struct Action
	{
		Action() = default;
		Action(Type type, sf::Vector2f position)
			: type(type), position(position)
		{}

		Type type;
		sf::Vector2f position;
	};
}
