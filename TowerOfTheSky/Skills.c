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
    SKILLID_nil,
    SKILLID_Channel_Mana,
    SKILLID_Wisdom,
    SKILLID_Focus,
    SKILLID_MAX,
};

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
    char name[32];
    char description[128];
    char effect_text[128];
    int level;

    float base_costs[MAX_COSTS];                   // Base costs for the skill
    float current_costs[MAX_COSTS];                // Current costs for the skill
    float cost_multipliers[MAX_COSTS];             // Multipliers for increasing costs
    float base_power_multiplier;                   // Base multiplier for power
    float current_power_multiplier;                // Current power multiplier

    float base_effect_value;                       // Base value of the effect
    float current_effect_value;                    // Current value of the effect

    EffectFunction apply_effect;                   // Function to apply the skill's effect

    ResourceID cost_resources[MAX_COSTS];           // Resources used for the cost
    ResourceID effect_resources[MAX_EFFECTS];       // Resources affected by the effect

    void (*level_up)(struct Skill* self, Resource resources[RESOURCEID_MAX]);
};

Resource* get_resource_from_resouce_list(ResourceID resource_ID, Resource resource_list[RESOURCEID_MAX])
{
	for (int i = 0; i < RESOURCEID_MAX; i++)
	{
		if (resource_list[i].resource_ID == resource_ID)
		{
			return & resource_list[i];
		}
	}
	return NULL;
}

void level_up_skill(Skill* self, Resource resources[RESOURCEID_MAX]) 
{
    // Spend resources and update costs
    for (int i = 0; i < MAX_COSTS; i++) 
    {
        ResourceID cost_id = self -> cost_resources[i];
        
        // Ensure the resource ID is valid
        if (cost_id != RESOURCEID_nil)
        {
            Resource* cost_resource = get_resource_from_resouce_list(cost_id, resources);
            
            // Check if enough resources are available to pay the cost
            if (cost_resource -> current >= self -> current_costs[i]) 
            {
                // Spend the resource
                cost_resource -> current -= self -> current_costs[i];

                // Increase the cost for the next level
                self -> current_costs[i] *= self -> cost_multipliers[i];

                // Increase the cost multiplier for the next level
                self -> cost_multipliers[i] *= cost_multiplier;
            } 
            else 
            {
                log("Not enough resources to level up the skill.\n");
                return;
            }
        }
    }

    Resource* effect_resources[MAX_EFFECTS] = {NULL};

    for (int i = 0; i < MAX_EFFECTS; i++) 
    {
        ResourceID effect_id = self -> effect_resources[i];
        
        // Ensure the effect resource ID is valid
        if (effect_id != RESOURCEID_nil) 
        {   

            for (int j = 0; j < RESOURCEID_MAX; j++)
            {
                if (resources[j].resource_ID == effect_id)
                {
                    effect_resources[i] = & resources[j];
                }
            }
        }
    }

    // Apply the specific effect of the Skill
    self -> apply_effect(effect_resources, self -> current_effect_value);

    // Calculate the new effect value
    self -> current_effect_value *= self -> current_power_multiplier;

    // Increase power multiplier for future levels
    self -> current_power_multiplier *= power_multiplier;

    // Level up the Skill
    self -> level++;
}

void set_skill_functions(Skill* skill) 
{
    switch (skill -> skill_ID) 
    {
        case SKILLID_Channel_Mana:
            skill -> apply_effect = apply_channel_mana_effect;
            skill -> level_up = level_up_skill;
            break;

        case SKILLID_Wisdom:
            skill -> apply_effect = apply_wisdom_effect;
            skill -> level_up = level_up_skill;
            break;

        case SKILLID_Focus:
            skill -> apply_effect = apply_focus_effect;
            skill -> level_up = level_up_skill;
            break;

        default:
            skill -> apply_effect = NULL;
            skill -> level_up = NULL;
            break;
    }
}

Skill channel_mana = 
{
    .skill_ID = SKILLID_Channel_Mana,
    .unlocked = false,
    .name = "Channel Mana",
    .description = "Channel your mana to Increase\nit's recovery speed.",
    .effect_text = "Base Mana / second",
    .level = 0,
    .base_costs = {25.0, 0.0},                                         // Base cost: 25 mana
    .current_costs = {25.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 2.0,                                          // Base effect value for mana per second buff
    .current_effect_value = 2.0,                                       // Initialize with base effect value
    .apply_effect = apply_channel_mana_effect,
    .cost_resources = {RESOURCEID_Mana},                               // Cost resource: Mana
    .effect_resources = {RESOURCEID_Mana},                             // Effect resource: Mana
    .level_up = level_up_skill,
};

Skill wisdom = 
{
    .skill_ID = SKILLID_Wisdom,
    .unlocked = false,
    .name = "Wisdom",
    .description = "Wisdom expands your mana reserves.",
    .effect_text = "Max Mana",
    .level = 0,
    .base_costs = {1.0, 0.0},                                           // Base cost: 1 Intellect
    .current_costs = {1.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 100.0,                                         // Base effect value for max mana increase
    .current_effect_value = 100.0,                                      // Initialize with base effect value
    .apply_effect = apply_wisdom_effect,
    .cost_resources = {RESOURCEID_Intellect},                           // Cost resource: Intellect
    .effect_resources = {RESOURCEID_Mana},                              // Effect resource: Mana
    .level_up = level_up_skill,
};

Skill focus = 
{
    .skill_ID = SKILLID_Focus,
    .unlocked = false,
    .name = "Focus",
    .description = "Passively generate Intellect.",
    .effect_text = "Base Intellect / second",
    .level = 0,
    .base_costs = {50.0, 1.0},                                           // Base cost: 50 Mana, 1 Intellect
    .current_costs = {50.0, 1.0},
    .cost_multipliers = {base_cost_multiplier, base_cost_multiplier},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 0.2,                                            // Base effect value
    .current_effect_value = 0.2,                                         // Initialize with base effect value
    .apply_effect = apply_focus_effect,
    .cost_resources = {RESOURCEID_Mana, RESOURCEID_Intellect},           // Cost resources: Mana and Intellect
    .effect_resources = {RESOURCEID_Intellect},                          // Effect resource: Intellect
    .level_up = level_up_skill,
};

Skill skills[SKILLID_MAX];

void load_skill_data()
{
    skills[SKILLID_Channel_Mana] = channel_mana;
    skills[SKILLID_Wisdom] = wisdom;
    skills[SKILLID_Focus] = focus;
}