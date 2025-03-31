#include "MultiplayerGameState.hpp"
#include "MusicPlayer.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>

sf::IpAddress GetAddressFromFile()
{
	std::ifstream input_file("ip.txt");
	std::string ip_address;
	if (input_file >> ip_address)
		return ip_address;

	std::ofstream output_file("ip.txt");
	std::string local_address = "127.0.0.1";
	output_file << local_address;
	return local_address;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool is_host)
	: State(stack, context)
	, m_world(*context.window, *context.fonts, *context.sounds, true)
	, m_window(*context.window)
	, m_texture_holder(*context.textures)
	, m_connected(false)
	, m_game_server(nullptr)
	, m_active_state(true)
	, m_has_focus(true)
	, m_host(is_host)
	, m_game_started(false)
	, m_client_timeout(sf::seconds(5.f))
	, m_time_since_last_packet(sf::seconds(0.f))
	, m_local_ready(false)
	, m_ready_players(0)
	, m_identifier(-1) 
{
	m_broadcast_text.setFont(context.fonts->Get(Font::kMain));
	m_broadcast_text.setPosition(1024.f / 2, 100.f);

	m_failed_connection_text.setFont(context.fonts->Get(Font::kMain));
	m_failed_connection_text.setCharacterSize(35);
	m_failed_connection_text.setFillColor(sf::Color::White);
	m_failed_connection_text.setString("Attempting to connect...");
	Utility::CentreOrigin(m_failed_connection_text);
	m_failed_connection_text.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

	m_window.clear(sf::Color::Black);
	m_window.draw(m_failed_connection_text);
	m_window.display();
	m_failed_connection_text.setString("Failed to connect to server");
	Utility::CentreOrigin(m_failed_connection_text);

	sf::IpAddress ip = is_host ? "127.0.0.1" : GetAddressFromFile();
	if (is_host)
		m_game_server = std::make_unique<GameServer>(sf::Vector2f(m_window.getSize()));

	if (m_socket.connect(ip, SERVER_PORT, sf::seconds(5.f)) == sf::TcpSocket::Done)
		m_connected = true;
	else
		m_failed_connection_clock.restart();

	m_socket.setBlocking(false);
	context.music->Play(MusicThemes::kMissionTheme);
}

void MultiplayerGameState::Draw()
{
	if (m_connected)
	{
		m_world.Draw();
		m_window.setView(m_window.getDefaultView());

		if (!m_broadcasts.empty())
			m_window.draw(m_broadcast_text);

		if (m_local_player_identifiers.size() < 2 && m_player_invitation_time < sf::seconds(0.5f))
		{
			m_window.draw(m_player_invitation_text);
		}
	}
	else
	{
		m_window.draw(m_failed_connection_text);
	}
}

bool MultiplayerGameState::Update(sf::Time dt)
{
	if (m_connected)
	{
		if (m_game_started)
			m_world.Update(dt);

		

		// Receive network packet
		sf::Packet packet;
		if (m_socket.receive(packet) == sf::Socket::Done)
		{
			std::cout << "[Multiplayer] Packet received.\n";
			m_time_since_last_packet = sf::seconds(0.f);
			sf::Int32 packet_type;
			packet >> packet_type;
			HandlePacket(packet_type, packet);
		}
		else if (m_time_since_last_packet > m_client_timeout)
		{
			std::cerr << "[Multiplayer] Connection timeout. Disconnecting.\n";
			m_connected = false;
			m_failed_connection_text.setString("Lost connection to server");
			Utility::CentreOrigin(m_failed_connection_text);
			m_failed_connection_clock.restart();
		}

		if (m_socket.receive(packet) == sf::Socket::Done)
{
    std::cout << "[Multiplayer] Packet received.\n";
    m_time_since_last_packet = sf::seconds(0.f);
    sf::Int32 packet_type;
    packet >> packet_type;
    HandlePacket(packet_type, packet);
}
else if (m_time_since_last_packet > m_client_timeout)
{
    std::cerr << "[Multiplayer] Connection timeout. Disconnecting.\n";
    m_connected = false;
    m_failed_connection_text.setString("Lost connection to server");
    Utility::CentreOrigin(m_failed_connection_text);
    m_failed_connection_clock.restart();
}

		// Send state
		if (m_tick_clock.getElapsedTime() > sf::seconds(1.f / TICK_RATE))
		{
			sf::Packet state;
			state << static_cast<sf::Int32>(Client::PacketType::kStateUpdate);
			state << static_cast<sf::Int32>(m_local_player_identifiers.size());

			for (auto id : m_local_player_identifiers)
			{
				if (Aircraft* a = m_world.GetAircraft(id))
				{
					state << id << a->getPosition().x << a->getPosition().y
						<< static_cast<sf::Int32>(a->GetHitPoints())
						<< static_cast<sf::Int32>(a->GetMissileAmmo());
				}
			}
			m_socket.send(state);
			m_tick_clock.restart();
		}

		m_time_since_last_packet += dt;
		UpdateBroadcastMessage(dt);
	}
	else if (m_failed_connection_clock.getElapsedTime() > sf::seconds(5.f))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return true;
}

bool MultiplayerGameState::HandleEvent(const sf::Event& event)
{
	CommandQueue& commands = m_world.GetCommandQueue();
	for (auto& pair : m_players)
	{
		pair.second->HandleEvent(event, commands);
	}
		



	if (event.type == sf::Event::KeyPressed)
	{
		// Enter: Ready logic (already there)
		if (event.key.code == sf::Keyboard::Return && m_connected && !m_game_started && !m_local_ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::kReadyNotice);
			packet << m_identifier;
			m_socket.send(packet);
			m_local_ready = true;

			std::cout << "[Multiplayer] Sent ReadyNotice with ID: " << m_identifier << "\n";
		}

		// Host shortcut (already there)
		if (m_host && event.key.code == sf::Keyboard::Return && !m_game_started && !m_local_ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::kReadyNotice);
			packet << m_identifier;
			m_socket.send(packet);
			m_local_ready = true;

			std::cout << "[Host] Host player sent ReadyNotice with ID: " << m_identifier << "\n";
		}

		//  ADD THIS: Pause Menu
		if (event.key.code == sf::Keyboard::Escape && m_game_started)
		{
			std::cout << "[Multiplayer] Pause triggered\n";
			RequestStackPush(StateID::kPause);  // Or use kNetworkPause if you have that too
		}



	}
	return true;
}

void MultiplayerGameState::OnActivate() { m_active_state = true; }

void MultiplayerGameState::OnDestroy()
{
	if (!m_host && m_connected)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PacketType::kQuit);
		m_socket.send(packet);
	}
}

void MultiplayerGameState::UpdateBroadcastMessage(sf::Time dt)
{
	if (m_broadcasts.empty()) return;

	m_broadcast_elapsed_time += dt;
	if (m_broadcast_elapsed_time > sf::seconds(2.f))
	{
		m_broadcasts.erase(m_broadcasts.begin());
		if (!m_broadcasts.empty())
		{
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
	}
}

void MultiplayerGameState::HandlePacket(sf::Int32 type, sf::Packet& packet)
{
	switch (static_cast<Server::PacketType>(type))
	{
	case Server::PacketType::kBroadcastMessage:
	{
		std::string message;
		packet >> message;
		m_broadcasts.push_back(message);
		if (m_broadcasts.size() == 1)
		{
			m_broadcast_text.setString(message);
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
		break;
	}
	case Server::PacketType::kSpawnSelf:
	{
		packet >> m_identifier;
		sf::Vector2f pos;
		packet >> pos.x >> pos.y;

		std::cout << "[Multiplayer] Spawning local aircraft with ID: " << m_identifier
			<< " at position (" << pos.x << ", " << pos.y << ")\n";

		auto* aircraft = m_world.AddAircraft(m_identifier);
		aircraft->setPosition(pos);

		// Center the camera on the aircraft
		sf::View view = GetContext().window->getDefaultView();
		view.setCenter(pos);
		GetContext().window->setView(view);

		m_players[m_identifier] = std::make_unique<Player>(&m_socket, m_identifier, GetContext().keys1);
		m_local_player_identifiers.push_back(m_identifier);
		break;
	}



	case Server::PacketType::kInitialState:
	{
		sf::Int32 count;
		packet >> m_world_height >> m_scroll_position >> count;

		for (sf::Int32 i = 0; i < count; ++i)
		{
			sf::Int32 id, hp, ammo;
			sf::Vector2f pos;
			packet >> id >> pos.x >> pos.y >> hp >> ammo;

			auto* aircraft = m_world.AddAircraft(id);
			aircraft->setPosition(pos);
			aircraft->SetHitpoints(hp);
			aircraft->SetMissileAmmo(ammo);

			m_players[id] = std::make_unique<Player>(&m_socket, id, nullptr);
		}
		break;
	}
	case Server::PacketType::kPlayerEvent:
	{
		sf::Int32 id, action;
		packet >> id >> action;
		m_players[id]->HandleNetworkEvent(static_cast<Action>(action), m_world.GetCommandQueue());
		break;
	}
	case Server::PacketType::kPlayerRealtimeChange:
	{
		sf::Int32 aircraft_identifier;
		sf::Int32 action;
		bool action_enabled;
		packet >> aircraft_identifier >> action >> action_enabled;

		auto itr = m_players.find(aircraft_identifier);
		if (itr != m_players.end())
		{
			itr->second->HandleNetworkRealtimeChange(static_cast<Action>(action), action_enabled);
		}
		break;
	}
	case Server::PacketType::kPlayerDisconnect:
	{
		sf::Int32 id;
		packet >> id;
		m_world.RemoveAircraft(id);
		m_players.erase(id);
		break;
	}
	case Server::PacketType::kGameReady:
		m_game_started = true;

		// Center camera on local player
		if (!m_local_player_identifiers.empty())
		{
			if (Aircraft* aircraft = m_world.GetAircraft(m_local_player_identifiers.front()))
			{
				sf::Vector2f pos = aircraft->getPosition();
				GetContext().window->setView(sf::View(
					sf::FloatRect(pos.x - 960.f, pos.y - 540.f, 1920.f, 1080.f)
				));
				std::cout << "[Multiplayer] Centering camera on Aircraft ID " << aircraft->GetIdentifier() << " at " << pos.x << ", " << pos.y << "\n";
			}
		}
		break;
	case Server::PacketType::kUpdateClientState:
	{
		float battlefield_position;
		sf::Int32 aircraft_count;
		packet >> battlefield_position >> aircraft_count;

		m_world.SetWorldScrollCompensation(m_world.GetViewBounds().top + m_world.GetViewBounds().height / battlefield_position);

		for (sf::Int32 i = 0; i < aircraft_count; ++i)
		{
			sf::Int32 id, hp, ammo;
			sf::Vector2f pos;
			packet >> id >> pos.x >> pos.y >> hp >> ammo;

			Aircraft* aircraft = m_world.GetAircraft(id);
			if (aircraft && std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), id) == m_local_player_identifiers.end())
			{
				// Interpolate position if not local
				sf::Vector2f interpolated = aircraft->getPosition() + (pos - aircraft->getPosition()) * 0.1f;
				aircraft->setPosition(interpolated);
				aircraft->SetHitpoints(hp);
				aircraft->SetMissileAmmo(ammo);
			}
		}
	}
	break;

	}
}
