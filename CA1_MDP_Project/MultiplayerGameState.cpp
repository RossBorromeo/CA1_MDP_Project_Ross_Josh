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
	, m_local_ready(false)                            //  NEW: Tracks local ready status
	, m_ready_players(0)                              //  NEW: Tracks total players ready
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

		sf::Packet packet;
		if (m_socket.receive(packet) == sf::Socket::Done)
		{
			m_time_since_last_packet = sf::seconds(0.f);
			sf::Int32 packet_type;
			packet >> packet_type;
			HandlePacket(packet_type, packet);
		}
		else if (m_time_since_last_packet > m_client_timeout)
		{
			m_connected = false;
			m_failed_connection_text.setString("Lost connection to server");
			Utility::CentreOrigin(m_failed_connection_text);
			m_failed_connection_clock.restart();
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
	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Return && m_connected && !m_game_started && !m_local_ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::kReadyNotice);
			packet << m_identifier;
			m_socket.send(packet);
			m_local_ready = true; // NEW: Prevents duplicate sending
		}
		else if (event.key.code == sf::Keyboard::Escape && m_game_started)
		{
			RequestStackPush(StateID::kPause);
		}
	}
	return true;
}

void MultiplayerGameState::OnActivate()
{
	m_active_state = true;
}

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

void MultiplayerGameState::HandlePacket(sf::Int32 packet_type, sf::Packet& packet)
{
	switch (static_cast<Server::PacketType>(packet_type))
	{
	case Server::PacketType::kBroadcastMessage:
	{
		std::string message;
		packet >> message;
		m_broadcasts.push_back(message);
		if (m_broadcasts.size() == 1)
		{
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
		break;
	}
	case Server::PacketType::kSpawnSelf:
	{
		sf::Int32 id;
		sf::Vector2f pos;
		packet >> id >> pos.x >> pos.y;

		Aircraft* aircraft = m_world.AddAircraft(id);
		aircraft->setPosition(pos);
		m_players[id] = std::make_unique<Player>(&m_socket, id, GetContext().keys1);
		m_local_player_identifiers.push_back(id);
		break;
	}
	case Server::PacketType::kInitialState:
	{
		sf::Int32 count;
		packet >> m_world_height;
		packet >> m_scroll_position;
		packet >> count;

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
	case Server::PacketType::kGameReady:
		m_game_started = true; //  NEW: triggers game start once all players are ready
		break;
	default:
		break;
	}
}
