#pragma once
#include "Skills.c"
#include "Upgrades.c"

typedef enum AbilityID AbilityID;

enum AbilityID
{
    ABILITYID_Nil,
    ABILITYID_Fire_Bolt,
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
    AbilityType ability_type;
    TargetType target_type;
    AbilityTag ability_tags;
    SkillRequirement required_skills;
    string ability_name;
    string description;

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