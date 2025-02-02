//D00238448:Joshua Thompson
//d00241095:Ross Borromeo
#pragma once
enum class ReceiverCategories
{
	kNone = 0,
	kScene = 1 << 0,
	kPlayerAircraft1 = 1 << 1,
	kPlayerAircraft2 = 1 << 2,
	kAlliedAircraft = 1 << 3,
	kEnemyAircraft = 1 << 4,
	kAlliedProjectile = 1 << 5,
	kEnemyProjectile = 1 << 6,
	/*kPickup = 1 << 7,*/
	kParticleSystem = 1 << 8,
	kSoundEffect = 1 << 9,

	kAircraft = kPlayerAircraft1 | kPlayerAircraft2 | kAlliedAircraft | kEnemyAircraft,
	kProjectile = kAlliedProjectile | kEnemyProjectile
};