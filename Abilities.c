#define MAX_COSTS 2 // Maximum number of costs for any ability

// Define base multipliers
const float base_cost_multiplier = 1.1;
const float base_power_multiplier = 1.05;
const float cost_multiplier = 1.005;  // Global multiplier for costs
const float power_multiplier = 1.0005;  // Multiplier for increasing power

typedef struct Resource Resource;

struct Resource 
{
    bool unlocked;
    float current;
    float max;
    float per_second;
};

Resource mana = {true, 0.0, 100.0, 10.0};
Resource intellect = {false, 0.0, 50.0, 0.0};

// Define the function pointer type for ability effects
typedef void (*EffectFunction)(Resource* resources[], float effect_value);

// Effect functions
void apply_channel_mana_effect(Resource* resources[], float effect_value) 
{
    if (resources[0] != NULL) 
    {
        resources[0] -> per_second += effect_value;  // Increase mana regen
    }
}

void apply_wisdom_effect(Resource* resources[], float effect_value)
{
    if (resources[0] != NULL) 
    {
        resources[0] -> max += effect_value;  // Increase max mana
    }
}

void apply_focus_effect(Resource* resources[], float effect_value) 
{
    if (resources[1] != NULL) 
    {
        resources[1] -> per_second += effect_value;  // Increase intellect regen
    }
}

typedef struct Ability Ability;

struct Ability 
{
    bool unlocked;
    int level;

    float base_costs[MAX_COSTS];         // Base costs for the ability
    float current_costs[MAX_COSTS];      // Current costs for the ability
    float cost_multipliers[MAX_COSTS];   // Multipliers for increasing costs
    float base_power_multiplier;         // Base multiplier for power
    float current_power_multiplier;      // Current power multiplier

    float base_effect_value;             // Base value of the effect
    float current_effect_value;          // Current value of the effect

    EffectFunction apply_effect;         // Function to apply the ability's effect

    void (*level_up)(struct Ability* self, Resource* resources[]);
};

void level_up_ability(Ability* self, Resource* resources[]) 
{
    // Apply the specific effect of the ability
    self -> apply_effect(resources, self -> current_effect_value);

    // Calculate the new effect value
    self -> current_effect_value = self -> base_effect_value * self->current_power_multiplier;

    // Spend resources and update costs
    for (int i = 0; i < MAX_COSTS; i++) 
    {
        if (resources[i] != NULL) 
        {
            if (resources[i] -> current >= self -> current_costs[i]) 
            {
                // Spend the resource
                resources[i] -> current -= self -> current_costs[i];

                // Increase the cost for the next level
                self -> current_costs[i] *= self -> cost_multipliers[i];

                // Increase the cost multiplier for the next level
                self -> cost_multipliers[i] *= cost_multiplier;
            } 
            else 
            {
                log("Not enough resources to level up the ability.\n");
            }
        } 
        else 
        {
            log("Error: resources[%i] is NULL\n", i);
        }
    }

    // Increase power multiplier for future levels
    self -> current_power_multiplier *= power_multiplier;

    // Level up the ability
    self -> level++;
}

Ability channel_mana = 
{
    .unlocked = true,
    .level = 0,
    .base_costs = {25.0, 0.0},                 // Base cost: 25 mana
    .current_costs = {25.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 2.0,                  // Base effect value for mana per second buff
    .current_effect_value = 2.0,               // Initialize with base effect value
    .apply_effect = apply_channel_mana_effect, // Set the specific effect function
    .level_up = level_up_ability
};

Ability wisdom = 
{
    .unlocked = false,
    .level = 0,
    .base_costs = {1.0, 0.0},                  // Base cost: 1 Intellect
    .current_costs = {1.0, 0.0},
    .cost_multipliers = {base_cost_multiplier, 1.0},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 100.0,                // Base effect value for max mana increase
    .current_effect_value = 100.0,             // Initialize with base effect value
    .apply_effect = apply_wisdom_effect,       // Set the specific effect function
    .level_up = level_up_ability
};

Ability focus = 
{
    .unlocked = false,
    .level = 0,
    .base_costs = {50.0, 1.0},               // Two different base costs
    .current_costs = {50.0, 1.0},
    .cost_multipliers = {base_cost_multiplier, base_cost_multiplier},
    .base_power_multiplier = base_power_multiplier,
    .current_power_multiplier = base_power_multiplier,
    .base_effect_value = 0.2,                // Base effect value
    .current_effect_value = 0.2,             // Initialize with base effect value
    .apply_effect = apply_focus_effect,      // Set the specific effect function
    .level_up = level_up_ability
};

//leveling up abilities
void level_up_channel_mana_if_unlocked() 
{
    if (channel_mana.unlocked && mana.current >= channel_mana.current_costs[0]) 
    {
        channel_mana.level_up(& channel_mana, (Resource*[]){& mana, NULL});
    }
}

void level_up_wisdom_if_unlocked() 
{
    if (wisdom.unlocked && intellect.current >= wisdom.current_costs[0]) 
    {
        wisdom.level_up(& wisdom, (Resource*[]){& intellect, NULL});
    }
}

void level_up_focus_if_unlocked() 
{
    if (focus.unlocked && mana.current >= focus.current_costs[0] && intellect.current >= focus.current_costs[1]) 
    {
        focus.level_up(& focus, (Resource*[]){& mana, & intellect});
    }
}