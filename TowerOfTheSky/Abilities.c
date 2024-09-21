#pragma once
#include "Skills.c"
#include "Upgrades.c"

typedef enum AbilityID AbilityID;

enum AbilityID
{
    ABILITYID_Nil,
    ABILITYID_Fire_Bolt,
    ABILITYID_MAX,
};

typedef enum AbilityType AbilityType;

enum AbilityType
{
    ABILITYTYPE_No_Ability_Type,
    ABILITYTYPE_Active,
    ABILITYTYPE_Passive,
    ABILITYTYPE_Triggered,
    ABILITYTYPE_Toggle,
};

typedef enum AbilityTag AbilityTag;

enum AbilityTag
{
    ABILITYTAG_No_Ability_Tag,
    ABILITYTAG_Spell,
    ABILITYTAG_Attack,
    ABILITYTAG_Melee,
    ABILITYTAG_Ranged,
    ABILITYTAG_Aura,
    ABILITYTAG_Curse,
    ABILITYTAG_Movement,
    ABILITYTAG_Buff,
    ABILITYTAG_MAX,
};

typedef enum TargetType TargetType;

enum TargetType
{
    TARGETTYPE_No_Target_Type,
    TARGETTYPE_Targeted,
    TARGETTYPE_Aura,
    TARGETTYPE_Area_Of_Effect,
    TARGETTYPE_Projectile,
    TARGETTYPE_Self,
};

typedef struct SkillRequirement SkillRequirement;

struct SkillRequirement
{
    AbilityID ability_ID;
    int required_level;
};

typedef struct Ability Ability;

struct Ability
{
    AbilityID ability_ID;
    AbilityType ability_type;
    TargetType target_type;
    AbilityTag ability_tags[ABILITYTAG_MAX];
    SkillRequirement required_skills[SKILLID_MAX];
    char ability_name[32];
    char description[128];
    bool unlocked;
    int damage;
    int damage_per_level;
    float ability_cooldown;
    float ability_duration; // 0 will be infinite Duration
    int base_resource_cost;
    int resource_cost_per_Level;
    int current_level;
    int max_level;
    int allocate_cost;
 };

void skill_requirement_check(SkillRequirement *skill_requirement, AbilityID ability_ID, int required_level) 
{
    skill_requirement -> ability_ID = ability_ID;
    skill_requirement -> required_level = required_level;
}

bool check_for_ability_tag(Ability *ability, AbilityTag tag) 
{
    for (int i = 0; i < ABILITYTAG_MAX; i++) 
    {
        if (ability -> ability_tags[i] == tag) 
        {
            return true;
        }
    }
    return false;
}

Ability fire_bolt = 
{
    .ability_ID = ABILITYID_Fire_Bolt,
    .ability_type = ABILITYTYPE_Active,
    .target_type =  TARGETTYPE_Projectile,
    .ability_tags = {ABILITYTAG_Spell, ABILITYTAG_Ranged},
    .required_skills = {},
    .ability_name = "Fire Bolt",
    .description = "A bolt of flame",
    .unlocked = true,
    .damage = 25,
    .damage_per_level = 1,
    .ability_cooldown = 0,
    .ability_duration = 0,
    .base_resource_cost = 5,
    .resource_cost_per_Level = 1,
    .current_level = 0,
    .max_level = 50,
    .allocate_cost = 1,
};