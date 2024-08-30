// Old Ability system for reference (not being used anymore)

// :Wizard Testing stuffs

const float base_cost_multiplier = 1.1;

const float base_power_multiplier = 1.05;

float cost_multiplier = 1.005;

float power_multiplier = 1.0005;

float no_second_cost = 0.0;

float no_second_resource = 0.0;

// Mana

bool mana_unlocked = true;

float current_mana = 0.0;

float max_mana = 100.0;

float mana_per_second = 10.0;

// Intellect

bool intellect_unlocked = false;

float current_intellect = 0.0;

float max_intellect = 50.0;

float intellect_per_second = 0.0;

// Channel Mana

bool channel_mana_known = true;

int channel_mana_level = 0;

const float channel_mana_base_cost = 25.0;

float channel_mana_current_cost = channel_mana_base_cost;

const float channel_mana_base_mana_per_second_buff = 2.0;

float channel_mana_current_mana_per_second_buff = channel_mana_base_mana_per_second_buff;

const float channel_mana_base_cost_multiplier = base_cost_multiplier;

float channel_mana_current_cost_multiplier = channel_mana_base_cost_multiplier;

const float channel_mana_base_power_multiplier = base_power_multiplier;

float channel_mana_current_power_multiplier = channel_mana_base_power_multiplier;

// Wisdom

bool wisdom_known = false;

int wisdom_level = 0;

const float wisdom_base_cost = 1;

float wisdom_current_cost = wisdom_base_cost;

const float wisdom_base_max_mana_buff = 100;

float wisdom_current_max_mana_buff = wisdom_base_max_mana_buff;

const float wisdom_base_cost_multiplier = base_cost_multiplier;

float wisdom_current_cost_multiplier = wisdom_base_cost_multiplier;

const float wisdom_base_power_multiplier = base_power_multiplier;

float wisdom_current_power_multiplier = wisdom_base_power_multiplier;

// Focus

bool focus_known = false;

int focus_level = 0;

const float focus_base_cost = 50;

const float focus_base_cost_2 = 1;

float focus_current_cost = focus_base_cost;

float focus_current_cost_2 = focus_base_cost_2;

const float focus_base_intellect_per_second_buff = 0.20;

float focus_current_intellect_per_second_buff = focus_base_intellect_per_second_buff;

const float focus_base_cost_multiplier = base_cost_multiplier;

float focus_current_cost_multiplier = focus_base_cost_multiplier;

const float focus_base_power_multiplier = base_power_multiplier;

float focus_current_power_multiplier = focus_base_power_multiplier;

typedef struct LevelUpParams LevelUpParams;

struct LevelUpParams
{
	float *statToBuff; // Which Stat to Increase
	float *buffAmount; // Increase by how much
	float *statBuffMultiplier; // Increase the buff multiplier
    float *currentResourceSpent; // Resource type spent
	float *currentResourceSpent_2; // Resource type spent 2
    float *currentCost; // Current Cost for resource 1
    float *currentCost2;  // Current Cost for resource 2
	float *costMultiplier; // Increase the cost multiplier
    int *level;
};

float ListOfCosts[] = {25, 30, 35};

void LevelUp(LevelUpParams *params)
{
	// Apply upgrade (buff the relevant attribute)
    *(params -> statToBuff) += *(params -> buffAmount);

    // Power up the upgrade
    *(params -> buffAmount) *= *(params -> statBuffMultiplier);

	// Increase power multiplier
	*(params -> statBuffMultiplier) *= power_multiplier;

    // Spend resources on the upgrade
    *(params -> currentResourceSpent) -= *(params -> currentCost);

	// Spend 2nd resource (if needed)
	if (*(params -> currentCost2) > 0)
	{
		*(params -> currentResourceSpent_2) -= *(params -> currentCost2);
	}

    // Increase the cost of the upgrade
    *(params -> currentCost) *= *(params -> costMultiplier);

	// Increase cost multiplier
	*(params -> costMultiplier) *= cost_multiplier;

    // Level up the upgrade
    (*(params -> level))++;
}

void LevelUpChannelMana()
{
    LevelUpParams params = 
	{
		& mana_per_second,
		& channel_mana_current_mana_per_second_buff,
		& channel_mana_current_power_multiplier,
		& current_mana, // resource spent 1
		& no_second_resource,
        & channel_mana_current_cost,
        & no_second_cost,
		& channel_mana_current_cost_multiplier,
        & channel_mana_level,
    };

    LevelUp(& params);
}

void LevelUpWisdom()
{
    LevelUpParams params = 
	{
		& max_mana,
		& wisdom_current_max_mana_buff,
		& wisdom_current_power_multiplier,
        & current_intellect, // resource spent 1
		& no_second_resource, 
        & wisdom_current_cost,
        & no_second_cost,
		& wisdom_current_cost_multiplier,
        & wisdom_level,
    };

    LevelUp(& params);
}

void LevelUpFocus()
{
    LevelUpParams params = 
	{
		& intellect_per_second,
		& focus_current_intellect_per_second_buff,
		& focus_current_power_multiplier,
        & current_mana, // resource spent 1
		& current_intellect, //resource spent 2
        & focus_current_cost,
        & focus_current_cost_2,
		& focus_current_cost_multiplier,
        & focus_level
    };

    LevelUp(& params);
}