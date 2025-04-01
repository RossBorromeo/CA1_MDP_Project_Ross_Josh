//Ross - D00241095 | Josh - D00238448
#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Config.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

class GameServer
{
public:
	explicit GameServer(sf::Vector2f battlefield_size);
	~GameServer();

	void NotifyPlayerSpawn(sf::Int32 aircraft_identifier);
	void NotifyPlayerRealtimeChange(sf::Int32 aircraft_identifier, sf::Int32 action, bool action_enabled);
	void NotifyPlayerEvent(sf::Int32 aircraft_identifier, sf::Int32 action);

private:
	struct RemotePeer
	{
		RemotePeer();
		sf::TcpSocket m_socket;
		sf::Time m_last_packet_time;
		std::vector<sf::Int32> m_aircraft_identifiers;
		bool m_ready;
		bool m_timed_out;
	};

	struct AircraftInfo
	{
		sf::Vector2f m_position;
		sf::Int32 m_hitpoints;
		sf::Int32 m_missile_ammo;
		std::map<sf::Int32, bool> m_realtime_actions;
	};

	using PeerPtr = std::unique_ptr<RemotePeer>;

private:
	void SetListening(bool enable);
	void ExecutionThread();
	void Tick();
	sf::Time Now() const;

	void HandleIncomingConnections();
	void HandleIncomingPackets();
	void HandleIncomingPackets(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout);
	void HandleDisconnections();

	void InformWorldState(sf::TcpSocket& socket);
	void BroadcastMessage(const std::string& message);
	void SendToAll(sf::Packet& packet);
	void UpdateClientState();

private:
	sf::Thread m_thread;
	sf::Clock m_clock;
	sf::TcpListener m_listener_socket;
	bool m_listening_state;
	sf::Time m_client_timeout;

	std::size_t m_max_connected_players;
	std::size_t m_connected_players;

	std::vector<PeerPtr> m_peers;
	sf::Int32 m_aircraft_identifier_counter;
	std::map<sf::Int32, AircraftInfo> m_aircraft_info;

	bool m_waiting_thread_end;
	bool m_game_started;  

	std::map<sf::Int32, bool> m_players_ready; 

	float m_world_height;
	sf::FloatRect m_battlefield_rect;
	float m_battlefield_scrollspeed;

	std::size_t m_aircraft_count;

	sf::Time m_last_spawn_time;
	sf::Time m_time_for_next_spawn;
};
