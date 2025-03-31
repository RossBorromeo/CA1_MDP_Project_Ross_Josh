#include "Aircraft.hpp"
#include "TextureID.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "DataTables.hpp"
#include "Projectile.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"

namespace {
	const std::vector<AircraftData> Table = InitializeAircraftData();
}

TextureID ToTextureID(AircraftType type) {
	switch (type) {
	case AircraftType::kBattleShip: return TextureID::kBattleShip;
	case AircraftType::kMeteor: return TextureID::kMeteor;
	case AircraftType::kAvenger: return TextureID::kAvenger;
	}
	return TextureID::kBattleShip;
}

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts)
	: Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type)
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture), Table[static_cast<int>(type)].m_texture_rect)
	, m_explosion(textures.Get(TextureID::kExplosion))
	, m_health_display(nullptr)
	, m_id_display(nullptr)
	, m_missile_display(nullptr)
	, m_distance_travelled(0.f)
	, m_directions_index(0)
	, m_fire_rate(1)
	, m_spread_level(1)
	, m_is_firing(false)
	, m_is_launching_missile(false)
	, m_fire_countdown(sf::Time::Zero)
	, m_missile_ammo(2)
	, m_is_marked_for_removal(false)
	, m_show_explosion(true)
	, m_explosion_began(false)
	, m_identifier(0)
	, m_invincibility_timer(sf::Time::Zero)
	, m_invincibility_duration(sf::seconds(1.5f))
	, m_respawn_position()
	
{
	m_explosion.SetFrameSize(sf::Vector2i(256, 256));
	m_explosion.SetNumFrames(16);
	m_explosion.SetDuration(sf::seconds(1));
	Utility::CentreOrigin(m_sprite);
	Utility::CentreOrigin(m_explosion);

	m_fire_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time dt) {
		CreateBullet(node, textures);
		};

	m_missile_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_missile_command.action = [this, &textures](SceneNode& node, sf::Time dt) {
		CreateProjectile(node, ProjectileType::kMissile, 0.f, 0.5f, textures);
		};

	std::string* health = new std::string("");
	std::unique_ptr<TextNode> health_display(new TextNode(fonts, *health));
	m_health_display = health_display.get();
	AttachChild(std::move(health_display));

	//ID display(Ross)
	if (GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft)) {
		std::string* player_id = new std::string("P" + std::to_string(m_identifier)); 
		std::unique_ptr<TextNode> id_display(new TextNode(fonts, *player_id));
		m_id_display = id_display.get();
		AttachChild(std::move(id_display));
	}


	if (Aircraft::GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft))

	{
		std::string* missile_ammo = new std::string("");
		std::unique_ptr<TextNode> missile_display(new TextNode(fonts, *missile_ammo));
		m_missile_display = missile_display.get();
		AttachChild(std::move(missile_display));
	}

	UpdateTexts();
}

unsigned int Aircraft::GetCategory() const
{
	if (IsAllied())
	{
		return static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
	}
	return static_cast<unsigned int>(ReceiverCategories::kEnemyAircraft);

}

int Aircraft::GetMissileAmmo() const {
	return m_missile_ammo;
}

void Aircraft::SetMissileAmmo(int ammo) {
	m_missile_ammo = ammo;
}

void Aircraft::IncreaseFireRate() {
	if (m_fire_rate < 5) ++m_fire_rate;
}

void Aircraft::IncreaseFireSpread() {
	if (m_spread_level < 3) ++m_spread_level;
}

void Aircraft::CollectMissile(unsigned int count) {
	m_missile_ammo += count;
}

void Aircraft::UpdateTexts() 
{
	m_health_display->SetString(std::to_string(GetHitPoints()) + "HP");
	m_health_display->setPosition(0.f, 50.f);
	m_health_display->setRotation(-getRotation());

	// Only update for player aircraft
	if (m_id_display) 
	{
		m_id_display->SetString("P" + std::to_string(m_identifier));
		m_id_display->setPosition(0.f, -50.f); // Position above aircraft
		m_id_display->setRotation(-getRotation());
	}

	/*if (m_missile_display) {
		m_missile_display->setPosition(0.f, 70.f);
		m_missile_display->SetString(m_missile_ammo == 0 ? "" : "M: " + std::to_string(m_missile_ammo));
	}*/
}

void Aircraft::UpdateMovementPattern(sf::Time dt) {
	const auto& directions = Table[static_cast<int>(m_type)].m_directions;
	if (!directions.empty()) {
		if (m_distance_travelled > directions[m_directions_index].m_distance) {
			m_directions_index = (m_directions_index + 1) % directions.size();
			m_distance_travelled = 0.f;
		}

		double radians = Utility::ToRadians(directions[m_directions_index].m_angle + 90.f);
		float vx = GetMaxSpeed() * std::cos(radians);
		float vy = GetMaxSpeed() * std::sin(radians);

		SetVelocity(vx, vy);
		m_distance_travelled += GetMaxSpeed() * dt.asSeconds();
	}
}

float Aircraft::GetMaxSpeed() const {
	return Table[static_cast<int>(m_type)].m_speed;
}

void Aircraft::Fire() {
	if (Table[static_cast<int>(m_type)].m_fire_interval != sf::Time::Zero) {
		m_is_firing = true;
	}
}

void Aircraft::LaunchMissile() {
	if (m_missile_ammo > 0) {
		m_is_launching_missile = true;
		--m_missile_ammo;
	}
}

void Aircraft::SetRespawnPosition(sf::Vector2f pos)
{
	m_respawn_position = pos;
}

sf::Vector2f Aircraft::GetRespawnPosition() const
{
	return m_respawn_position;
}

void Aircraft::CreateBullet(SceneNode& node, const TextureHolder& textures) const {
	ProjectileType type = IsAllied() ? ProjectileType::kAlliedBullet : ProjectileType::kEnemyBullet;
	switch (m_spread_level) {
	case 1: CreateProjectile(node, type, 0.0f, 0.5f, textures); break;
	case 2:
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	case 3:
		CreateProjectile(node, type, 0.0f, 0.5f, textures);
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	}
}

void Aircraft::CreateProjectile(SceneNode& node, ProjectileType type, float x_offset, float y_offset, const TextureHolder& textures) const {
	auto projectile = std::make_unique<Projectile>(type, textures);
	sf::Vector2f offset(x_offset * m_sprite.getGlobalBounds().width, y_offset * m_sprite.getGlobalBounds().height);
	sf::Vector2f velocity(0, projectile->GetMaxSpeed());

	float sign = IsAllied() ? -1.f : 1.f;
	projectile->setPosition(GetWorldPosition() + offset * sign);
	projectile->SetVelocity(velocity * sign);
	node.AttachChild(std::move(projectile));
}

sf::FloatRect Aircraft::GetBoundingRect() const {
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

int Aircraft::GetIdentifier() 
{
	return m_identifier;
}

void Aircraft::SetIdentifier(int identifier) 
{
	m_identifier = identifier;  

	if (m_id_display) 
	{
		m_id_display->SetString("P" + std::to_string(m_identifier)); 
	}
}

bool Aircraft::IsMarkedForRemoval() const {
	return IsDestroyed() && (m_explosion.IsFinished() || !m_show_explosion);
}

void Aircraft::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(IsDestroyed() && m_show_explosion ? static_cast<const sf::Drawable&>(m_explosion) : m_sprite, states);
}

void Aircraft::UpdateCurrent(sf::Time dt, CommandQueue& commands) {
	UpdateInvincibility(dt);

	if (IsDestroyed()) {
		m_explosion.Update(dt);
		if (!m_explosion_began) {
			SoundEffect soundEffect = (Utility::RandomInt(2) == 0) ? SoundEffect::kExplosion1 : SoundEffect::kExplosion2;
			PlayLocalSound(commands, soundEffect);

			if (!IsAllied()) {
				Command command;
				command.category = static_cast<int>(ReceiverCategories::kNetwork);
				command.action = DerivedAction<NetworkNode>([pos = GetWorldPosition()](NetworkNode& node, sf::Time) {
					node.NotifyGameAction(GameActions::kEnemyExplode, pos);
					});
				commands.Push(command);
			}
			m_explosion_began = true;
		}
		return;
	}

	Entity::UpdateCurrent(dt, commands);
	UpdateTexts();
	UpdateMovementPattern(dt);
	UpdateRollAnimation();
	CheckProjectileLaunch(dt, commands);
}

void Aircraft::CheckProjectileLaunch(sf::Time dt, CommandQueue& commands) {
	if (!IsAllied()) Fire();

	if (m_is_firing && m_fire_countdown <= sf::Time::Zero) {
		PlayLocalSound(commands, IsAllied() ? SoundEffect::kEnemyGunfire : SoundEffect::kAlliedGunfire);
		commands.Push(m_fire_command);
		m_fire_countdown += Table[static_cast<int>(m_type)].m_fire_interval / (m_fire_rate + 1.f);
		m_is_firing = false;
	}
	else if (m_fire_countdown > sf::Time::Zero) {
		m_fire_countdown -= dt;
		m_is_firing = false;
	}

	if (m_is_launching_missile) {
		PlayLocalSound(commands, SoundEffect::kLaunchMissile);
		commands.Push(m_missile_command);
		m_is_launching_missile = false;
	}
}

bool Aircraft::IsAllied() const {
	return m_type == AircraftType::kBattleShip;
}

void Aircraft::Remove() {
	Entity::Remove();
	m_show_explosion = false;
}

void Aircraft::UpdateRollAnimation() {
	if (Table[static_cast<int>(m_type)].m_has_roll_animation) {
		sf::IntRect textureRect = Table[static_cast<int>(m_type)].m_texture_rect;
		if (GetVelocity().x < 0.f) textureRect.left += textureRect.width;
		else if (GetVelocity().x > 0.f) textureRect.left += 2 * textureRect.width;
		m_sprite.setTextureRect(textureRect);
	}
}

void Aircraft::PlayLocalSound(CommandQueue& commands, SoundEffect effect) {
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kSoundEffect);
	command.action = DerivedAction<SoundNode>(
		[effect, pos = GetWorldPosition()](SoundNode& node, sf::Time) {
			node.PlaySound(effect, pos);
		});
	commands.Push(command);
}

bool Aircraft::IsInvincible() const {
	return m_invincibility_timer > sf::Time::Zero;
}

void Aircraft::StartInvincibility() {
	m_invincibility_timer = m_invincibility_duration;
}

void Aircraft::UpdateInvincibility(sf::Time dt) {
	if (m_invincibility_timer > sf::Time::Zero) {
		m_invincibility_timer -= dt;
	}
}
