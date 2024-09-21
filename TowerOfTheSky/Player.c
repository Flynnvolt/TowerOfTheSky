#pragma once
#include "Abilities.c"
#include "Upgrades.c"

typedef struct Player Player;

struct Player
{
    char name[32];
    Resource resource_list[RESOURCEID_MAX];
    Skill skill_list[SKILLID_MAX];
    Ability ability_list[ABILITYID_MAX];
    Upgrade upgrade_list[UPGRADEID_MAX];
    int level;
    int max_level;
};

Player hero_default =
{
    .name = "Hero",
    .resource_list = {},
    .skill_list = {},
    .ability_list = {},
    .upgrade_list = {},
    .level = 1,
    .max_level = 100,
};