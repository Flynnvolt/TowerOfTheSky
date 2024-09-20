#pragma once
#include "Abilities.c"
#include "Limits.c"
#include "Entities.c"
#include "Sprites.c"
#include "AnimationData.c"

typedef struct Projectile Projectile;

struct Projectile 
{
    bool is_active;

    AnimationInfo animation;  

    Vector2 position;
    Vector2 velocity;

    Vector2 target_position; 

    float distance_traveled;
    float max_distance;
	float time_alive;
	float max_time_alive;

    float speed;              
    float rotation;
    float damage;
    float scale;
    float radius;
	float x_remainder;
	float y_remainder;

	Entity *source_entity; // Caster of the Projectile
};