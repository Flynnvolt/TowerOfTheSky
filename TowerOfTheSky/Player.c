#pragma once
#include "Abilities.c"
#include "Upgrades.c"

typedef struct Player Player;

struct Player
{
    char name[32];
    Entity player;
    Resource resource_list[RESOURCEID_MAX];
    Skill skill_list[SKILLID_MAX];
    Ability ability_list[ABILITYID_MAX];
    Upgrade all_upgrades[UPGRADEID_MAX];
    Upgrade known_upgrades[UPGRADEID_MAX];
    Upgrade upgrade_list[UPGRADEID_MAX];
    int level;
    int max_level;
};

Player player_hero = 
{
    .player = 
    {
        .entity_ID = ENTITY_Player,
        .sprite_ID = SPRITE_Player,
        .is_valid = true,
        .health = 100,
        .max_health = 100,
        .health_regen = 2,
        .speed = 75,
        .pos = {0, 0}, 
    },
    
    .name = "Hero",
    .level = 1,
    .max_level = 100,
};