#pragma once
#include "Entities.c"

typedef enum EnemyState EnemyState;

enum EnemyState
{
	ENEMYSTATE_idle,
	ENEMYSTATE_sleep,
	ENEMYSTATE_patrol,
	ENEMYSTATE_combat,
	ENEMYSTATE_flee,
};

typedef struct EnemyLogic EnemyLogic;

struct EnemyLogic
{
	EnemyState enemy_state;
	Vector2 roam_direction;
	float damage;
	float attack_cooldown;
	float current_attack_cooldown;
	float attack_range;
	float aggro_range;
	float roam_time;
	float idle_time;
	float flee_time;
};

typedef struct Enemy Enemy;

struct Enemy
{
	Entity enemy_entity;
	EnemyLogic enemy_logic;
};