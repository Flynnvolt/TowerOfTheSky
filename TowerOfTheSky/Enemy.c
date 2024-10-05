#pragma once
#include "Entities.c"

typedef enum EnemyID EnemyID;

enum EnemyID
{
	ENEMYID_nil,
	ENEMYID_slime,
	ENEMYID_MAX,
};

typedef enum EnemyState EnemyState;

enum EnemyState
{
	ENEMYSTATE_idle,
	ENEMYSTATE_sleep,
	ENEMYSTATE_patrol,
	ENEMYSTATE_combat,
	ENEMYSTATE_knockback,
	ENEMYSTATE_flee,
	ENEMYSTATE_MAX,
};

typedef struct EnemyLogic EnemyLogic;

struct EnemyLogic
{
	EnemyState enemy_state;
	Vector2 direction;
	EnemyID enemy_ID;
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

Enemy slime_defaults = 
{
    .enemy_entity = 
    {
        .entity_ID = ENTITY_Enemy,
        .sprite_ID = SPRITE_Slime,
        .health = 50,
        .max_health = 50,
        .health_regen = 0.2,
        .speed = 20,
        .is_valid = true, 
        .current_floor = 0 
    },
    
    .enemy_logic = 
    {
        .enemy_ID = ENEMYID_slime,
        .damage = 5,
        .attack_cooldown = 1,
        .current_attack_cooldown = 0,
        .attack_range = 30,
        .aggro_range = 100,
        .enemy_state = ENEMYSTATE_idle,
        .direction = (Vector2){0, 0},
        .roam_time = 0,
        .idle_time = 0,
        .flee_time = 0
    }
};

Enemy enemy_defaults[ENEMYID_MAX];

void load_enemy_defaults() 
{
    enemy_defaults[ENEMYID_slime] = slime_defaults;
}

Enemy get_enemy_defaults(EnemyID enemy_id) 
{
    if (enemy_id <= ENEMYID_nil || enemy_id >= ENEMYID_MAX) 
    {
        log("Invalid EnemyID: %i", enemy_id);
        return (Enemy){0};
    }

    return enemy_defaults[enemy_id];
}