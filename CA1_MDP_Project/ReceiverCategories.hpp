#pragma once

namespace ReceiverCategories
{
	enum Type
	{
		kNone = 0,
		kScene = 1 << 0,
		kPlayerAircraft = 1 << 1,
		kAlliedAircraft = 1 << 2,
		kEnemyAircraft = 1 << 3,
		kPickup = 1 << 4,
		kProjectile = 1 << 5,
		kPlayer1 = 1 << 6, // New category for Player 1
		kPlayer2 = 1 << 7,  // New category for Player 2
		kPlayerProjectile,
		kEnemyProjectile,
	};
}