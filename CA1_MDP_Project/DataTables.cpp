//D00238448:Joshua Thompson
//D00241095:Ross Borromeo

#include "DataTables.hpp"
#include "AircraftType.hpp"
#include "ProjectileType.hpp"
#include "PickupType.hpp"
#include "Aircraft.hpp"
#include "ParticleType.hpp"
#include "NetworkProtocol.hpp"

std::vector<AircraftData> InitializeAircraftData()
{
    std::vector<AircraftData> data(static_cast<int>(AircraftType::kAircraftCount));

    data[static_cast<int>(AircraftType::kBattleShip)].m_hitpoints = 100;
    data[static_cast<int>(AircraftType::kBattleShip)].m_speed = 210.f;
    data[static_cast<int>(AircraftType::kBattleShip)].m_fire_interval = sf::seconds(1);
    data[static_cast<int>(AircraftType::kBattleShip)].m_texture = TextureID::kBattleShip;
    data[static_cast<int>(AircraftType::kBattleShip)].m_texture_rect = sf::IntRect(0, 0, 84, 63);

    //data[static_cast<int>(AircraftType::kBattleShip)].m_has_roll_animation = true;

    data[static_cast<int>(AircraftType::kMeteor)].m_hitpoints = 50;
    data[static_cast<int>(AircraftType::kMeteor)].m_speed = 125.f;
    data[static_cast<int>(AircraftType::kMeteor)].m_fire_interval = sf::Time::Zero;
    data[static_cast<int>(AircraftType::kMeteor)].m_texture = TextureID::kMeteor;
    data[static_cast<int>(AircraftType::kMeteor)].m_texture_rect = sf::IntRect(0, 0, 90, 90);
    data[static_cast<int>(AircraftType::kMeteor)].m_has_roll_animation = false;

    //AI for Asteroid
    data[static_cast<int>(AircraftType::kMeteor)].m_directions.emplace_back(Direction(+45.f, 80.f));
    data[static_cast<int>(AircraftType::kMeteor)].m_directions.emplace_back(Direction(-45.f, 160.f));
    data[static_cast<int>(AircraftType::kMeteor)].m_directions.emplace_back(Direction(+45.f, 80.f));

    //AI for Meteor
    data[static_cast<int>(AircraftType::kAvenger)].m_hitpoints = 120;
    data[static_cast<int>(AircraftType::kAvenger)].m_speed = 75.f;
    data[static_cast<int>(AircraftType::kMeteor)].m_fire_interval = sf::Time::Zero;
    data[static_cast<int>(AircraftType::kAvenger)].m_texture = TextureID::kAvenger;
    data[static_cast<int>(AircraftType::kAvenger)].m_texture_rect = sf::IntRect(0, 0, 95, 95);
    data[static_cast<int>(AircraftType::kAvenger)].m_has_roll_animation = false;


    data[static_cast<int>(AircraftType::kAvenger)].m_directions.emplace_back(Direction(+45.f, 50.f));
    data[static_cast<int>(AircraftType::kAvenger)].m_directions.emplace_back(Direction(0.f, 50.f));
    data[static_cast<int>(AircraftType::kAvenger)].m_directions.emplace_back(Direction(-45.f, 100.f));
    data[static_cast<int>(AircraftType::kAvenger)].m_directions.emplace_back(Direction(0.f, 50.f));
    data[static_cast<int>(AircraftType::kAvenger)].m_directions.emplace_back(Direction(45.f, 50.f));

    return data;
}

std::vector<ProjectileData> InitializeProjectileData()
{
    std::vector<ProjectileData> data(static_cast<int>(ProjectileType::kProjectileCount));
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_damage = 10;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_speed = 300;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_texture_rect = sf::IntRect(175, 64, 3, 14);


    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_damage = 10;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_speed = 300;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_texture_rect = sf::IntRect(175, 64, 3, 14);


    data[static_cast<int>(ProjectileType::kMissile)].m_damage = 200;
    data[static_cast<int>(ProjectileType::kMissile)].m_speed = 150;
    data[static_cast<int>(ProjectileType::kMissile)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kMissile)].m_texture_rect = sf::IntRect(160, 64, 15, 32);

    return data;
}

////std::vector<PickupData> InitializePickupData()
////{
////    std::vector<PickupData> data(static_cast<int>(PickupType::kPickupCount));
////    data[static_cast<int>(PickupType::kHealthRefill)].m_texture = TextureID::kEntities;
////    data[static_cast<int>(PickupType::kHealthRefill)].m_texture_rect = sf::IntRect(0, 64, 40, 40);
////    data[static_cast<int>(PickupType::kHealthRefill)].m_action = [](Aircraft& a)
////        {
////            a.Repair(25);
////        };
////
////    data[static_cast<int>(PickupType::kMissileRefill)].m_texture = TextureID::kEntities;
////    data[static_cast<int>(PickupType::kMissileRefill)].m_texture_rect = sf::IntRect(40, 64, 40, 40);
////
////    data[static_cast<int>(PickupType::kMissileRefill)].m_action = std::bind(&Aircraft::CollectMissile, std::placeholders::_1, 3);
////
////    data[static_cast<int>(PickupType::kFireSpread)].m_texture = TextureID::kEntities;
////    data[static_cast<int>(PickupType::kFireSpread)].m_texture_rect = sf::IntRect(80, 64, 40, 40);
////    data[static_cast<int>(PickupType::kFireSpread)].m_action = std::bind(&Aircraft::IncreaseFireSpread, std::placeholders::_1);
////
////    data[static_cast<int>(PickupType::kFireRate)].m_texture = TextureID::kEntities;
////    data[static_cast<int>(PickupType::kFireRate)].m_texture_rect = sf::IntRect(120, 64, 40, 40);
////    data[static_cast<int>(PickupType::kFireRate)].m_action = std::bind(&Aircraft::IncreaseFireRate, std::placeholders::_1);
////
////    return data;
//}

std::vector<ParticleData> InitializeParticleData()
{
    std::vector<ParticleData> data(static_cast<int>(ParticleType::kParticleCount));

    data[static_cast<int>(ParticleType::kPropellant)].m_color = sf::Color(255, 255, 50);
    data[static_cast<int>(ParticleType::kPropellant)].m_lifetime = sf::seconds(0.5f);

    data[static_cast<int>(ParticleType::kSmoke)].m_color = sf::Color(50, 50, 50);
    data[static_cast<int>(ParticleType::kSmoke)].m_lifetime = sf::seconds(2.5f);

    return data;
}


std::map<int, SpawnPoint> InitializeSpawnPoints()
{
    std::map<int, SpawnPoint> data;

    const int MAX_CONNECTIONS = 15;
    const int SPAWN_COLUMNS = 5;
    const int SPAWN_ROWS = 3;
    const float WINDOW_WIDTH = 1920.f;
    const float WINDOW_HEIGHT = 1080.f;
    const float HORIZONTAL_PADDING = 100.f;
    const float VERTICAL_START_Y = WINDOW_HEIGHT - 300.f;
    const float VERTICAL_SPACING = 120.f;

    const float available_width = WINDOW_WIDTH - 2 * HORIZONTAL_PADDING;
    const float spacing_x = available_width / (SPAWN_COLUMNS - 1);

    int spawn_id = 0;
    for (int row = 0; row < SPAWN_ROWS; ++row)
    {
        float y = VERTICAL_START_Y - row * VERTICAL_SPACING;

        for (int col = 0; col < SPAWN_COLUMNS; ++col)
        {
            if (spawn_id >= MAX_CONNECTIONS)
                break;

            float x = HORIZONTAL_PADDING + col * spacing_x;

            data[spawn_id] = SpawnPoint{ x, y };
            ++spawn_id;
        }
    }

    return data;
}




