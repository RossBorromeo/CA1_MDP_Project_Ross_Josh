//D00238448:Joshua Thompson
//D00241095:Ross Borromeo

#include "MultiplayerGameState.hpp"
#include "MusicPlayer.hpp"
#include "Utility.hpp"
#include "GameOverState.hpp"


#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

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

	m_player_id_text.setFont(context.fonts->Get(Font::kMain));
	m_player_id_text.setCharacterSize(30);
	m_player_id_text.setFillColor(sf::Color::White);
	m_player_id_text.setString("Your Player ID: ???");
	m_player_id_text.setPosition(1024.f / 2.f, 140.f); // Below the broadcast text
	Utility::CentreOrigin(m_player_id_text);

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

		if (!m_game_started)
			m_window.draw(m_player_id_text);

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
		//  ONLY update game world if server says the game has started
		if (m_game_started)
			m_world.Update(dt);

		//  Still handle network packets regardless of game_started
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
			m_failed_connection_text.setString("Lost connection to the server");
			Utility::CentreOrigin(m_failed_connection_text);
			m_failed_connection_clock.restart();
		}

		m_time_since_last_packet += dt;


		if (m_game_started && !m_game_over_shown) //  ONLY check after game starts
		{
			bool all_local_dead = true;
			for (sf::Int32 id : m_local_player_identifiers)
			{
				Aircraft* ac = m_world.GetAircraft(id);
				if (ac && !ac->IsMarkedForRemoval() && ac->GetHitPoints() > 0)
				{
					all_local_dead = false;
					break;
				}
			}

			if (all_local_dead)
			{
				m_game_over_shown = true;
				RequestStackPush(StateID::kGameOver);
				DisableAllRealtimeActions();
			}
		}


		
		UpdateBroadcastMessage(dt);

		
		m_player_invitation_time += dt;
		if (m_player_invitation_time > sf::seconds(1.f))
			m_player_invitation_time = sf::Time::Zero;

	
		if (m_game_started)
		{
			// Remove dead aircraft
			bool found_local_plane = false;
			for (auto itr = m_players.begin(); itr != m_players.end();)
			{
				if (std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), itr->first) != m_local_player_identifiers.end())
					found_local_plane = true;

				if (!m_world.GetAircraft(itr->first))
				{
					itr = m_players.erase(itr);
					if (m_players.empty())
						RequestStackPush(StateID::kGameOver);
				}
				else ++itr;
			}
			if (!found_local_plane)
				RequestStackPush(StateID::kGameOver);

			// Realtime input
			if (m_active_state && m_has_focus)
			{
				CommandQueue& commands = m_world.GetCommandQueue();
				for (auto& pair : m_players)
					pair.second->HandleRealtimeInput(commands);
			}

			// Network input
			CommandQueue& commands = m_world.GetCommandQueue();
			for (auto& pair : m_players)
				pair.second->HandleRealtimeNetworkInput(commands);

			// Game actions
			GameActions::Action game_action;
			while (m_world.PollGameAction(game_action))
			{
				sf::Packet p;
				p << static_cast<sf::Int32>(Client::PacketType::kGameEvent);
				p << static_cast<sf::Int32>(game_action.type);
				p << game_action.position.x << game_action.position.y;
				m_socket.send(p);
			}

			// Position updates
			if (m_tick_clock.getElapsedTime() > sf::seconds(1.f / TICK_RATE))
			{
				sf::Packet p;
				p << static_cast<sf::Int32>(Client::PacketType::kStateUpdate);
				p << static_cast<sf::Int32>(m_local_player_identifiers.size());

				for (sf::Int32 id : m_local_player_identifiers)
				{
					if (Aircraft* ac = m_world.GetAircraft(id))
					{
						p << id << ac->getPosition().x << ac->getPosition().y
							<< static_cast<sf::Int32>(ac->GetHitPoints())
							<< static_cast<sf::Int32>(ac->GetMissileAmmo());
					}
				}
				m_socket.send(p);
				m_tick_clock.restart();
			}
		}
	}
	else if (m_failed_connection_clock.getElapsedTime() >= sf::seconds(5.f))
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
	
		if (event.key.code == sf::Keyboard::Return && m_connected && !m_game_started && !m_local_ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::kReadyNotice);
			packet << m_identifier;
			m_socket.send(packet);
			m_local_ready = true;

			std::cout << "[Multiplayer] Sent ReadyNotice with ID: " << m_identifier << "\n";
		}

		

		
		if (event.key.code == sf::Keyboard::Escape && m_game_started)
		{
			std::cout << "[Multiplayer] Pause triggered\n";
			RequestStackPush(StateID::kPause);  
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

void MultiplayerGameState::DisableAllRealtimeActions()
{
	m_active_state = false;
	for (sf::Int32 identifier : m_local_player_identifiers)
	{
		m_players[identifier]->DisableAllRealtimeActions();
	}
}

void MultiplayerGameState::UpdateBroadcastMessage(sf::Time elapsed_time)
{
	if (m_broadcasts.empty()) return;

	m_broadcast_elapsed_time += elapsed_time;
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
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
		
	}
	break;
	case Server::PacketType::kSpawnSelf:
	{
		sf::Int32 aircraft_identifier;
		sf::Vector2f aircraft_position;
		packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y;
		m_identifier = aircraft_identifier;

		std::stringstream ss;
		ss << "Your Player ID: " << m_identifier;
		m_player_id_text.setString(ss.str());
		Utility::CentreOrigin(m_player_id_text);

		Aircraft* aircraft = m_world.AddAircraft(aircraft_identifier);
		aircraft->setPosition(aircraft_position);
		m_players[aircraft_identifier].reset(new Player(&m_socket, aircraft_identifier, GetContext().keys1));
		m_local_player_identifiers.push_back(aircraft_identifier);
		
	}
	break;

	case Server::PacketType::kPlayerConnect:
	{
		sf::Int32 aircraft_identifier;
		sf::Vector2f aircraft_position;
		packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y;

		Aircraft* aircraft = m_world.AddAircraft(aircraft_identifier);
		aircraft->setPosition(aircraft_position);
		m_players[aircraft_identifier].reset(new Player(&m_socket, aircraft_identifier, nullptr));
	}
	break;


	case Server::PacketType::kInitialState:
	{
		sf::Int32 aircraft_count;
		float world_height, current_scroll;
		packet >> world_height >> current_scroll;

		m_world.SetWorldHeight(world_height);
		m_world.SetCurrentBattleFieldPosition(current_scroll);
		packet >> aircraft_count;

		for (sf::Int32 i = 0; i < aircraft_count; ++i)
		{
			sf::Int32 aircraft_identifier;
			sf::Int32 hitpoints;
			sf::Int32 missile_ammo;
			sf::Vector2f aircraft_position;
			packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y >> hitpoints >> missile_ammo;

			Aircraft* aircraft = m_world.AddAircraft(aircraft_identifier);
			aircraft->setPosition(aircraft_position);
			aircraft->SetHitpoints(hitpoints);
			aircraft->SetMissileAmmo(missile_ammo);

			m_players[aircraft_identifier].reset(new Player(&m_socket, aircraft_identifier, nullptr));
		}
		
		
	}
	break;

	case Server::PacketType::kPlayerEvent:
	{
		sf::Int32 aircraft_identifier;
		sf::Int32 action;
		packet >> aircraft_identifier >> action;

		auto itr = m_players.find(aircraft_identifier);
		if (itr != m_players.end())
		{
			itr->second->HandleNetworkEvent(static_cast<Action>(action), m_world.GetCommandQueue());
		}
	}

	break;

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
		
	}
	break;

	case Server::PacketType::kPlayerDisconnect:
	{
		sf::Int32 id;
		packet >> id;
		m_world.RemoveAircraft(id);
		m_players.erase(id);
		
	}
	break;

	case Server::PacketType::kGameReady:
	{
		m_game_started = true;
		m_broadcasts.clear(); 

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
	}
	break;

	case Server::PacketType::kDestroyAircraft:
	{
		sf::Int32 id;
		packet >> id;
		m_world.RemoveAircraft(id); //  marks for removal
		m_players.erase(id);        //  optional: clean up input handler
	}
	break;




	case Server::PacketType::kSpawnEnemy:
	{
		float height;
		sf::Int32 type;
		float relative_x;
		packet >> type >> height >> relative_x;

		m_world.AddEnemy(static_cast<AircraftType>(type), relative_x, height);
		m_world.SortEnemies();
	}
	break;

	

	case Server::PacketType::kMissionSuccess:
	{
		bool is_winner = false;
		for (sf::Int32 id : m_local_player_identifiers)
		{
			Aircraft* ac = m_world.GetAircraft(id);
			if (ac && !ac->IsMarkedForRemoval() && ac->GetHitPoints() > 0)
			{
				is_winner = true;
				break;
			}
		}

		if (is_winner)
		{
			GameOverState::s_last_message = "Asteroids Avoided";
			RequestStackPush(StateID::kGameOver);


		}
		else
		{
			GameOverState::s_last_message = "Ship Destroyed";
			RequestStackPush(StateID::kGameOver);

		}
	}
	break;



	case Server::PacketType::kUpdateClientState:
	{
		float current_world_position;
		sf::Int32 aircraft_count;
		packet >> current_world_position >> aircraft_count;

		float current_view_position = m_world.GetViewBounds().top + m_world.GetViewBounds().height;

		//Set the world's scroll compensation according to whether the view is behind or ahead
		m_world.SetWorldScrollCompensation(current_view_position / current_world_position);


		for (sf::Int32 i = 0; i < aircraft_count; ++i)
		{
			sf::Vector2f aircraft_position;
			sf::Int32 aircraft_identifier;
			sf::Int32 hitpoints;
			sf::Int32 ammo;
			packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y >> hitpoints >> ammo;

			Aircraft* aircraft = m_world.GetAircraft(aircraft_identifier);
			bool is_local_plane = std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), aircraft_identifier) != m_local_player_identifiers.end();
			if (aircraft && !is_local_plane)
			{
				sf::Vector2f interpolated_position = aircraft->getPosition() + (aircraft_position - aircraft->getPosition()) * 0.1f;
				aircraft->setPosition(interpolated_position);
				aircraft->SetHitpoints(hitpoints);
				aircraft->SetMissileAmmo(ammo);
			}

		}
	}
	break;

	}
}
