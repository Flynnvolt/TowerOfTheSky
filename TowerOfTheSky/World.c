#pragma once
#include "Limits.c"
#include "UXState.c"
#include "Item.c"
#include "Enemy.c"
#include "Floor.c"
#include "Projectile.c"
#include "Player.c"

typedef struct Projectile Projectile;

typedef struct World World;

struct World
{
	float64 time_elapsed;

	UXState ux_state;

	float inventory_alpha;

	float inventory_alpha_target;

	Player player;

	int active_projectiles;

	Projectile projectiles[MAX_PROJECTILES];

	ItemData items[ITEM_MAX];
	
	int current_floor;

	float floor_cooldown;

	int active_floors;

	FloorData floors[MAX_FLOOR_COUNT];
};

World* world = 0;