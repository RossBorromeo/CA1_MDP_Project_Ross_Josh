#include "GameServer.hpp"
#include <SFML/Network/Packet.hpp>
#include "NetworkProtocol.hpp"
#include <SFML/System/Sleep.hpp>
#include "Utility.hpp"
#include "PickupType.hpp"
#include "AircraftType.hpp"

#include <iostream>

GameServer::GameServer(sf::Vector2f battlefield_size)
    : m_thread(&GameServer::ExecutionThread, this)
    , m_listening_state(false)
    , m_client_timeout(sf::seconds(5.f))
    , m_max_connected_players(15)
    , m_connected_players(0)
    , m_world_height(5000.f)
    , m_battlefield_rect(0.f, m_world_height - battlefield_size.y, battlefield_size.x, battlefield_size.y)
    , m_battlefield_scrollspeed(-50.f)
    , m_aircraft_count(0)
    , m_peers(1)
    , m_aircraft_identifier_counter(1)
    , m_waiting_thread_end(false)
    , m_last_spawn_time(sf::Time::Zero)
    , m_time_for_next_spawn(sf::seconds(5.f))
    , m_game_started(false)
{
    m_listener_socket.setBlocking(false);
    m_peers[0].reset(new RemotePeer());
    m_thread.launch();
}

GameServer::~GameServer()
{
    m_waiting_thread_end = true;
    m_thread.wait();
}

void GameServer::NotifyPlayerSpawn(sf::Int32 aircraft_identifier)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerConnect);
    packet << aircraft_identifier << m_aircraft_info[aircraft_identifier].m_position.x << m_aircraft_info[aircraft_identifier].m_position.y;
    SendToAll(packet);
}

void GameServer::NotifyPlayerRealtimeChange(sf::Int32 aircraft_identifier, sf::Int32 action, bool action_enabled)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerRealtimeChange);
    packet << aircraft_identifier;
    packet << action;
    packet << action_enabled;
    SendToAll(packet);
}

void GameServer::NotifyPlayerEvent(sf::Int32 aircraft_identifier, sf::Int32 action)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerEvent);
    packet << aircraft_identifier;
    packet << action;
    SendToAll(packet);
}

void GameServer::SetListening(bool enable)
{
    //Check is the server is already listening
    if (enable)
    {
        if (!m_listening_state)
        {
            m_listening_state = (m_listener_socket.listen(SERVER_PORT) == sf::TcpListener::Done);
        }
    }
    else
    {
        m_listener_socket.close();
        m_listening_state = false;
    }
}

void GameServer::ExecutionThread()
{
    SetListening(true);

    // Self-spawn for host
    m_aircraft_info[m_aircraft_identifier_counter].m_position = sf::Vector2f(m_battlefield_rect.width / 2.f, m_battlefield_rect.top + m_battlefield_rect.height / 2.f);
    m_aircraft_info[m_aircraft_identifier_counter].m_hitpoints = 100;
    m_aircraft_info[m_aircraft_identifier_counter].m_missile_ammo = 2;
    m_peers[0]->m_aircraft_identifiers.emplace_back(m_aircraft_identifier_counter);

    // Build spawn packet and send to host
    sf::Packet self_spawn;
    self_spawn << static_cast<sf::Int32>(Server::PacketType::kSpawnSelf);
    self_spawn << m_aircraft_identifier_counter;
    self_spawn << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
    self_spawn << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

    m_peers[0]->m_socket.send(self_spawn);
    m_peers[0]->m_ready = true;
    m_peers[0]->m_last_packet_time = Now();

    m_aircraft_count++;
    m_connected_players++;
    m_aircraft_identifier_counter++;


    sf::Time frame_rate = sf::seconds(1.f / 60.f);
    sf::Time tick_rate = sf::seconds(1.f / 20.f);
    sf::Time frame_time = sf::Time::Zero;
    sf::Time tick_time = sf::Time::Zero;
    sf::Clock frame_clock, tick_clock;

    while (!m_waiting_thread_end)
    {
        HandleIncomingConnections();
        HandleIncomingPackets();

        frame_time += frame_clock.restart();
        tick_time += tick_clock.restart();

        if (m_game_started)
        {
            while (frame_time >= frame_rate)
            {
                m_battlefield_rect.top += m_battlefield_scrollspeed * frame_rate.asSeconds();
                frame_time -= frame_rate;
            }

            while (tick_time >= tick_rate)
            {
                Tick();
                tick_time -= tick_rate;
            }
        }
        else
        {
            sf::Packet waiting;
            waiting << static_cast<sf::Int32>(Server::PacketType::kBroadcastMessage);
            waiting << "Waiting for players to press Ready...";
            SendToAll(waiting);
        }

        sf::sleep(sf::milliseconds(50));
    }
}


void GameServer::Tick()
{
    UpdateClientState();

    //Check if the game is over = all planes postion.y < offset

    bool all_aircraft_done = true;
    for (const auto& current : m_aircraft_info)
    {
        //As long as one player has not crossed the finish line the game is still live
        if (current.second.m_position.y > 0.f)
        {
            all_aircraft_done = false;
            break;
        }
    }

    if (all_aircraft_done)
    {
        sf::Packet mission_success_packet;
        mission_success_packet << static_cast<sf::Int32>(Server::PacketType::kMissionSuccess);
        SendToAll(mission_success_packet);
    }

    //Remove aircraft that have been destroyed
    for (auto itr = m_aircraft_info.begin(); itr != m_aircraft_info.end();)
    {
        if (itr->second.m_hitpoints <= 0)
        {
            m_aircraft_info.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }

    //Check if it is time to spawn enemies
    if (Now() >= m_time_for_next_spawn + m_last_spawn_time)
    {
        //Not going to spawn enemies near the end
        if (m_battlefield_rect.top > 600.f)
        {
            std::size_t enemy_count = 1 + Utility::RandomInt(2);
            float spawn_centre = static_cast<float>(Utility::RandomInt(500) - 250);

            //If there is only one enemy it is at spawn_centre
            float plane_distance = 0.f;
            float next_spawn_position = spawn_centre;

            //If there are two enemies they should be centred on the spawn centre
            if (enemy_count == 2)
            {
                plane_distance = static_cast<float>(150 + Utility::RandomInt(250));
                next_spawn_position = spawn_centre - plane_distance / 2.f;
            }

            //Send the spawn packets to the clients
            for (std::size_t i = 0; i < enemy_count; ++i)
            {
                sf::Packet packet;
                packet << static_cast<sf::Int32>(Server::PacketType::kSpawnEnemy);
                packet << static_cast<sf::Int32>(1 + Utility::RandomInt(static_cast<int>(AircraftType::kAircraftCount) - 1));

                packet << m_world_height - m_battlefield_rect.top + 500;
                packet << next_spawn_position;

                next_spawn_position += plane_distance / 2.f;
                SendToAll(packet);
            }
            m_last_spawn_time = Now();
            m_time_for_next_spawn = sf::milliseconds(2000 + Utility::RandomInt(6000));
        }
    }
}

sf::Time GameServer::Now() const
{
    return m_clock.getElapsedTime();
}

void GameServer::HandleIncomingPackets()
{
    bool detected_timeout = false;

    for (PeerPtr& peer : m_peers)
    {
        if (peer->m_ready)
        {
            sf::Packet packet;
            while (peer->m_socket.receive(packet) == sf::Socket::Done)
            {
                //Interpret the packet and react to it
                HandleIncomingPackets(packet, *peer, detected_timeout);

                peer->m_last_packet_time = Now();
                packet.clear();
            }

            if (Now() > peer->m_last_packet_time + m_client_timeout)
            {
                peer->m_timed_out = true;
                detected_timeout = true;
            }

        }
    }

    if (detected_timeout)
    {
        HandleDisconnections();
    }
}

void GameServer::HandleIncomingPackets(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout)
{
    sf::Int32 packet_type;
    packet >> packet_type;

    switch (static_cast<Client::PacketType> (packet_type))
    {
    case Client::PacketType::kQuit:
    {
        receiving_peer.m_timed_out = true;
        detected_timeout = true;
    }
    break;

    case Client::PacketType::kPlayerEvent:
    {
        sf::Int32 aircraft_identifier;
        sf::Int32 action;
        packet >> aircraft_identifier >> action;
        NotifyPlayerEvent(aircraft_identifier, action);
    }
    break;

    case Client::PacketType::kPlayerRealtimeChange:
    {
        sf::Int32 aircraft_identifier;
        sf::Int32 action;
        bool action_enabled;
        packet >> aircraft_identifier >> action >> action_enabled;
        NotifyPlayerRealtimeChange(aircraft_identifier, action, action_enabled);
    }
    break;

    case Client::PacketType::kReadyNotice:
    {
        sf::Int32 id;
        packet >> id;
        m_players_ready[id] = true;

        if (!m_game_started && m_players_ready.size() == m_connected_players)
        {
            m_game_started = true;
            sf::Packet ready_packet;
            ready_packet << static_cast<sf::Int32>(Server::PacketType::kGameReady);
            SendToAll(ready_packet);
            BroadcastMessage("Game Started");
            SetListening(false);
        }
    }
    break;



  /*  case Client::PacketType::kRequestCoopPartner:
    {
        receiving_peer.m_aircraft_identifiers.emplace_back(m_aircraft_identifier_counter);
        m_aircraft_info[m_aircraft_identifier_counter].m_position = sf::Vector2f(m_battlefield_rect.width / 2, m_battlefield_rect.top + m_battlefield_rect.height / 2);
        m_aircraft_info[m_aircraft_identifier_counter].m_hitpoints = 100;
        m_aircraft_info[m_aircraft_identifier_counter].m_missile_ammo = 2;

        sf::Packet request_packet;
        request_packet << static_cast<sf::Int32>(Server::PacketType::kAcceptCoopPartner);
        request_packet << m_aircraft_identifier_counter;
        request_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        request_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

        receiving_peer.m_socket.send(request_packet);
        m_aircraft_count++;

        // Tell everyone else about the new plane
        sf::Packet notify_packet;
        notify_packet << static_cast<sf::Int32>(Server::PacketType::kPlayerConnect);
        notify_packet << m_aircraft_identifier_counter;
        notify_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        notify_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

        for (PeerPtr& peer : m_peers)
        {
            if (peer.get() != &receiving_peer && peer->m_ready)
            {

                peer->m_socket.send(notify_packet);
            }
        }

        m_aircraft_identifier_counter++;
    }*/
    //break;

    case Client::PacketType::kStateUpdate:
    {
        sf::Int32 num_aircraft;
        packet >> num_aircraft;

        for (sf::Int32 i = 0; i < num_aircraft; ++i)
        {
            sf::Int32 aircraft_identifier;
            sf::Int32 aircraft_hitpoints;
            sf::Int32 missile_ammo;
            sf::Vector2f aircraft_position;
            packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y >> aircraft_hitpoints >> missile_ammo;
            m_aircraft_info[aircraft_identifier].m_position = aircraft_position;
            m_aircraft_info[aircraft_identifier].m_hitpoints = aircraft_hitpoints;
            m_aircraft_info[aircraft_identifier].m_missile_ammo = missile_ammo;
        }
    }
    break;

    case Client::PacketType::kGameEvent: 
    {
        sf::Int32 action;
        float x;
        float y;

        packet >> action;
        packet >> x;
        packet >> y;

        //Enemy explodes, with a certain probability, drop a pickup
        //To avoid multiple messages only listen to the first peer (host)
        if (action == GameActions::kEnemyExplode && Utility::RandomInt(3) == 0 && &receiving_peer == m_peers[0].get())
        {
            sf::Packet packet;
            packet << x;
            packet << y;

            SendToAll(packet);
        }
    }
    }
}

void GameServer::HandleIncomingConnections()
{
    if (!m_listening_state)
        return;

    //  Ensure the slot exists before accessing it
    if (m_connected_players >= m_peers.size())
        m_peers.emplace_back(std::make_unique<RemotePeer>());

    if (m_listener_socket.accept(m_peers[m_connected_players]->m_socket) == sf::TcpListener::Done)
    {
        auto& peer = m_peers[m_connected_players];

        m_aircraft_info[m_aircraft_identifier_counter].m_position = sf::Vector2f(
            m_battlefield_rect.width / 2.f,
            m_battlefield_rect.top + m_battlefield_rect.height / 2.f
        );
        m_aircraft_info[m_aircraft_identifier_counter].m_hitpoints = 100;
        m_aircraft_info[m_aircraft_identifier_counter].m_missile_ammo = 2;

        sf::Packet packet;
        packet << static_cast<sf::Int32>(Server::PacketType::kSpawnSelf);
        packet << m_aircraft_identifier_counter;
        packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

        peer->m_aircraft_identifiers.emplace_back(m_aircraft_identifier_counter);

        BroadcastMessage("New player");
        InformWorldState(peer->m_socket);
        NotifyPlayerSpawn(m_aircraft_identifier_counter++);

        peer->m_socket.send(packet);
        peer->m_ready = true;
        peer->m_last_packet_time = Now();

        m_aircraft_count++;
        m_connected_players++;

        if (m_connected_players >= m_max_connected_players)
            SetListening(false);
    }
}


void GameServer::HandleDisconnections()
{
    for (auto itr = m_peers.begin(); itr != m_peers.end();)
    {
        RemotePeer* peer = itr->get();

        if (peer->m_timed_out)
        {
            // Notify and erase all aircraft owned by this peer
            for (sf::Int32 id : peer->m_aircraft_identifiers)
            {
                // Inform all clients about disconnection
                sf::Packet disconnect_packet;
                disconnect_packet << static_cast<sf::Int32>(Server::PacketType::kPlayerDisconnect);
                disconnect_packet << id;
                SendToAll(disconnect_packet);

                // Remove aircraft info
                m_aircraft_info.erase(id);

                // Remove ready flag
                m_players_ready.erase(id);
            }

            m_aircraft_count -= peer->m_aircraft_identifiers.size();
            m_connected_players--;

            // Erase peer
            itr = m_peers.erase(itr);

            // Reopen listener if game hasn't started and room available
            if (!m_game_started && m_connected_players < m_max_connected_players)
            {
                m_peers.emplace_back(PeerPtr(new RemotePeer()));
                SetListening(true);
            }

            BroadcastMessage("A player has disconnected.");
        }
        else
        {
            ++itr;
        }
    }
}


void GameServer::InformWorldState(sf::TcpSocket& socket)
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::PacketType::kInitialState);
    packet << m_world_height << m_battlefield_rect.top + m_battlefield_rect.height;
    packet << static_cast<sf::Int32>(m_aircraft_count);

    for (std::size_t i = 0; i < m_connected_players; ++i)
    {
        if (m_peers[i]->m_ready)
        {
            for (sf::Int32 identifier : m_peers[i]->m_aircraft_identifiers)
            {
                packet << identifier << m_aircraft_info[identifier].m_position.x << m_aircraft_info[identifier].m_position.y << m_aircraft_info[identifier].m_hitpoints << m_aircraft_info[identifier].m_missile_ammo;
            }
        }
    }

    socket.send(packet);
}

void GameServer::BroadcastMessage(const std::string& message)
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::PacketType::kBroadcastMessage);
    packet << message;
    for (std::size_t i = 0; i < m_connected_players; ++i)
    {
        if (m_peers[i]->m_ready)
        {
            m_peers[i]->m_socket.send(packet);
        }
    }
}

void GameServer::SendToAll(sf::Packet& packet)
{
    std::size_t sent_count = 0;

    for (const PeerPtr& peer : m_peers)
    {
        if (peer->m_ready && !peer->m_timed_out)
        {
            if (peer->m_socket.send(packet) == sf::Socket::Done)
            {
                ++sent_count;
            }
        }
    }

    std::cout << "[GameServer] Sent packet to " << sent_count << " active peers\n";
}


void GameServer::UpdateClientState()
{
    sf::Packet update_client_state_packet;
    update_client_state_packet << static_cast<sf::Int32>(Server::PacketType::kUpdateClientState);
    update_client_state_packet << static_cast<float>(m_battlefield_rect.top + m_battlefield_rect.height);

    sf::Int32 aircraft_sent = 0;

    // Reserve space for aircraft count (to be filled later)
    update_client_state_packet << aircraft_sent; // placeholder at index 2

    std::vector<sf::Packet> aircraft_packets;

    for (std::map<sf::Int32, AircraftInfo>::const_iterator it = m_aircraft_info.begin(); it != m_aircraft_info.end(); ++it)
    {
        const sf::Int32 id = it->first;
        const AircraftInfo& info = it->second;

        if (info.m_hitpoints > 0)
        {
            sf::Packet p;
            p << id << info.m_position.x << info.m_position.y << info.m_hitpoints << info.m_missile_ammo;
            aircraft_packets.push_back(p);
            ++aircraft_sent;
        }
    }

    // Repack cleanly now that we know aircraft count
    // Rebuild packet with accurate count and all aircraft
    sf::Packet final_packet;
    final_packet << static_cast<sf::Int32>(Server::PacketType::kUpdateClientState);
    final_packet << static_cast<float>(m_battlefield_rect.top + m_battlefield_rect.height);
    final_packet << aircraft_sent;

    for (std::vector<sf::Packet>::const_iterator pkt_it = aircraft_packets.begin(); pkt_it != aircraft_packets.end(); ++pkt_it)
    {
        final_packet.append(pkt_it->getData(), pkt_it->getDataSize());
    }

    SendToAll(final_packet);
}



//It is essential to set the sockets to non-blocking - m_socket.setBlocking(false)
//otherwise the server will hang waiting to read input from a connection

GameServer::RemotePeer::RemotePeer() : m_ready(false), m_timed_out(false)
{
    m_socket.setBlocking(false);
}