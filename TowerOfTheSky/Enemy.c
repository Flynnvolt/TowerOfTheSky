#pragma once

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
	bool state_setup;
	float roam_time;
	Vector2 roam_direction;
	float idle_time;
	float flee_time;
};