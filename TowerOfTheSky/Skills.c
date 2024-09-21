#pragma once
#include "Resources.c"

#define MAX_COSTS 2    // Maximum number of costs for any skill
#define MAX_EFFECTS 2  // Maximum number of effect resources for any skill

// Base multipliers
const float base_cost_multiplier = 1.1;
const float base_power_multiplier = 1.05;
const float cost_multiplier = 1.005;  // Multiplier for increasing base cost Multiplier
const float power_multiplier = 1.0005;  //  Multiplier for increasing base power Multiplier

typedef enum SkillID SkillID;

enum SkillID
{
    SKILLID_Channel_Mana,
    SKILLID_wisdom,
    SKILLID_focus,
    SKILLID_MAX,
};

// Define the function pointer type for skill effects
typedef void (*EffectFunction)(Resource* resources[], float effect_value);

// skill effect application
void apply_channel_mana_effect(Resource* resources[], float effect_value) 
{
    if (resources[0] != NULL) 
    {
        resources[0] -> per_second += effect_value;   // Increase mana regen
    }
}

void apply_wisdom_effect(Resource* resources[], float effect_value)
{
    if (resources[0] != NULL) 
    {
        resources[0] -> max += effect_value;          // Increase max mana
    }
}

void apply_focus_effect(Resource* resources[], float effect_value) 
{
    if (resources[0] != NULL) 
    {
        resources[0] -> per_second += effect_value;   // Increase intellect regen
    }
}

typedef struct Skill Skill;

struct Skill
{
    SkillID skill_ID;
    bool unlocked;
    int level;

    float base_costs[MAX_COSTS];                   // Base costs for the skill
    float current_costs[MAX_COSTS];                // Current costs for the skill
    float cost_multipliers[MAX_COSTS];             // Multipliers for increasing costs
    float base_power_multiplier;                   // Base multiplier for power
    float current_power_multiplier;                // Current power multiplier

    float base_effect_value;                       // Base value of the effect
    float current_effect_value;                    // Current value of the effect

    EffectFunction apply_effect;                   // Function to apply the skill's effect

    Resource* cost_resources[MAX_COSTS];           // Resources used for the cost
    Resource* effect_resources[MAX_EFFECTS];       // Resources affected by the effect

    void (*level_up)(struct Skill* self);
};

void level_up_skill(Skill* self) 
{
    // Spend resources and update costs
    for (int i = 0; i < MAX_COSTS; i++) 
    {
        if (self -> cost_resources[i] != NULL) 
        {
            if (self -> cost_resources[i] -> current >= self -> current_costs[i]) 
            {
                // Spend the resource
                self -> cost_resources[i] -> current -= self -> current_costs[i];

                // Increase the cost for the next level
                self -> current_costs[i] *= self -> cost_multipliers[i];

                // Increase the cost multiplier for the next level
                self -> cost_multipliers[i] *= cost_multiplier;
            } 
            else 
            {
                log("Not enough resources to level up the skill.\n");
                return; // Exit early if not enough resources
            }
        }
    }

    // Apply the specific effect of the Skill
    self -> apply_effect(self -> effect_resources, self -> current_effect_value);

    // Calculate the new effect value
    self -> current_effect_value *= self -> current_power_multiplier;

    // Increase power multiplier for future levels
    self -> current_power_multiplier *= power_multiplier;

    // Level up the Skill
    self -> level++;
}

Skill channel_mana = 
{
    .skill_ID = SKILLID_Channel_Mana,
    .unlocked = false,
    .level = 0,
    .base_costs = {25.0, 0.0},                         // Base cost: 25 mana
    .current_costs = {25.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 2.0,                          // Base effect value for mana per second buff
    .current_effect_value = 2.0,                       // Initialize with base effect value
    .apply_effect = apply_channel_mana_effect,
    .cost_resources = {& mana, NULL},                  // Cost resource: Mana
    .effect_resources = {& mana, NULL},                // Effect resource: Mana
    .level_up = level_up_skill,
};

Skill wisdom = 
{
    .skill_ID = SKILLID_wisdom,
    .unlocked = false,
    .level = 0,
    .base_costs = {1.0, 0.0},                          // Base cost: 1 Intellect
    .current_costs = {1.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 100.0,                        // Base effect value for max mana increase
    .current_effect_value = 100.0,                     // Initialize with base effect value
    .apply_effect = apply_wisdom_effect,
    .cost_resources = {& intellect, NULL},             // Cost resource: Intellect
    .effect_resources = {& mana, NULL},                // Effect resource: Mana
    .level_up = level_up_skill,
};

Skill focus = 
{
    .skill_ID = SKILLID_focus,
    .unlocked = false,
    .level = 0,
    .base_costs = {50.0, 1.0},                         // Base cost: 50 Mana, 1 Intellect
    .current_costs = {50.0, 1.0},
    .cost_multipliers = {base_cost_multiplier, base_cost_multiplier},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 0.2,                          // Base effect value
    .current_effect_value = 0.2,                       // Initialize with base effect value
    .apply_effect = apply_focus_effect,
    .cost_resources = {& mana, & intellect},           // Cost resources: Mana and Intellect
    .effect_resources = {& intellect, NULL},           // Effect resource: Intellect
    .level_up = level_up_skill,
};

Skill skills[SKILLID_MAX];

void load_skill_data()
{
    skills[SKILLID_Channel_Mana] = channel_mana;
    skills[SKILLID_wisdom] = wisdom;
    skills[SKILLID_focus] = focus;
}

// Leveling up skills

void level_up_channel_mana_if_unlocked() 
{
    if (channel_mana.unlocked == true) 
    {
        channel_mana.level_up(& channel_mana);
    }
}

void level_up_wisdom_if_unlocked() 
{
    if (wisdom.unlocked == true) 
    {
        wisdom.level_up(& wisdom);
    }
}

void level_up_focus_if_unlocked() 
{
    if (focus.unlocked == true) 
    {
        focus.level_up(& focus);
    }
}