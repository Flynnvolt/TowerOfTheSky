#define MAX_COSTS 2    // Maximum number of costs for any ability
#define MAX_EFFECTS 2  // Maximum number of effect resources for any ability

// Base multipliers
const float base_cost_multiplier = 1.1;
const float base_power_multiplier = 1.05;
const float cost_multiplier = 1.005;  // Multiplier for increasing base cost Multiplier
const float power_multiplier = 1.0005;  //  Multiplier for increasing base power Multiplier

typedef struct Resource Resource;

struct Resource 
{
    bool unlocked;
    float current;
    float max;
    float per_second;
};

// Resources
Resource mana = {true, 100.0, 100.0, 10.0};
Resource intellect = {false, 0.0, 50.0, 0.5};

// Define the function pointer type for ability effects
typedef void (*EffectFunction)(Resource* resources[], float effect_value);

// Ability effect application
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

typedef struct Ability Ability;

struct Ability 
{
    bool unlocked;
    int level;

    float base_costs[MAX_COSTS];                   // Base costs for the ability
    float current_costs[MAX_COSTS];                // Current costs for the ability
    float cost_multipliers[MAX_COSTS];             // Multipliers for increasing costs
    float base_power_multiplier;                   // Base multiplier for power
    float current_power_multiplier;                // Current power multiplier

    float base_effect_value;                       // Base value of the effect
    float current_effect_value;                    // Current value of the effect

    EffectFunction apply_effect;                   // Function to apply the ability's effect

    Resource* cost_resources[MAX_COSTS];           // Resources used for the cost
    Resource* effect_resources[MAX_EFFECTS];       // Resources affected by the effect

    void (*level_up)(struct Ability* self);
};

void level_up_ability(Ability* self) 
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
                log("Not enough resources to level up the ability.\n");
                return; // Exit early if not enough resources
            }
        }
    }

    // Apply the specific effect of the ability
    self -> apply_effect(self -> effect_resources, self -> current_effect_value);

    // Calculate the new effect value
    self -> current_effect_value *= self -> current_power_multiplier;

    // Increase power multiplier for future levels
    self -> current_power_multiplier *= power_multiplier;

    // Level up the ability
    self -> level++;
}

Ability channel_mana = 
{
    .unlocked = true,
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
    .level_up = level_up_ability
};

Ability wisdom = 
{
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
    .level_up = level_up_ability
};

Ability focus = 
{
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
    .level_up = level_up_ability
};

typedef enum AbilityList AbilityList;

enum AbilityList
{
    Ability_channel_mana,
    Ability_wisdom,
    Ability_focus,
};

// Leveling up abilities
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