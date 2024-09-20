#pragma once
#include "Sprites.c"

typedef enum EntityID EntityID;

enum EntityID
{
	ENTITY_Nil,
	ENTITY_Player,
	ENTITY_Enemy,
	ENTITY_MAX,
};

typedef struct Entity Entity;

struct Entity
{
	EntityID entity_ID;
	bool is_valid;
	SpriteID sprite_ID;
	Vector2 pos;
	float health;
	float max_health;
	float health_regen;
	float speed;
	bool is_immortal;
	float x_remainder;
	float y_remainder;
	int current_floor;
};