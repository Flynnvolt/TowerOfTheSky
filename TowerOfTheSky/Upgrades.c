#pragma once
#include "Abilities.c"

typedef enum UpgradeID UpgradeID;

enum UpgradeID
{
    UPGRADEID_nil,
    UPGRADEID_Unlock_Mana,
    UPGRADEID_Unlock_Magic,
    UPGRADEID_Unlock_Arcana,
    UPGRADEID_Unlock_Fire_Bolt,
    UPGRADEID_Multishot,
    UPGRADEID_MAX,
};

typedef struct Upgrade Upgrade;

struct Upgrade
{
    UpgradeID upgrade_ID;
    UpgradeID upgrades_required[UPGRADEID_MAX];
    UpgradeID upgrades_revealed[UPGRADEID_MAX];
    AbilityID abilities_unlocked[ABILITYID_MAX];
    SkillID skills_unlocked[SKILLID_MAX];
    ResourceID resources_unlocked[RESOURCEID_MAX];
    bool known;
    bool unlocked;
    char name[32];
    char description[128];
    int level;
};

Upgrade unlock_mana = 
{
    .upgrade_ID = UPGRADEID_Unlock_Mana,
    .upgrades_revealed = {},
    .abilities_unlocked = {},
    .upgrades_required = {},
    .skills_unlocked = {SKILLID_Channel_Mana},
    .resources_unlocked = {RESOURCEID_Mana},
    .known = true,
    .unlocked = false,
    .name = "Study Mana",
    .description = "",
    .level = 0,
};

Upgrade unlock_magic = 
{
    .upgrade_ID = UPGRADEID_Unlock_Magic,
    .upgrades_revealed = {UPGRADEID_Unlock_Arcana, UPGRADEID_Unlock_Fire_Bolt},
    .upgrades_required = {UPGRADEID_Unlock_Mana},
    .abilities_unlocked = {},
    .skills_unlocked = {SKILLID_Wisdom},
    .resources_unlocked = {RESOURCEID_Intellect},
    .known = false,
    .unlocked = false,
    .name = "Study Magic",
    .description = "",
    .level = 0,
};

Upgrade unlock_arcana = 
{
    .upgrade_ID = UPGRADEID_Unlock_Arcana,
    .upgrades_revealed = {UPGRADEID_Multishot},
    .upgrades_required = {UPGRADEID_Unlock_Magic},
    .abilities_unlocked = {},
    .skills_unlocked = {SKILLID_Focus},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .name = "Study Arcana",
    .description = "",
    .level = 0,
};

Upgrade unlock_firebolt = 
{
    .upgrade_ID = UPGRADEID_Unlock_Fire_Bolt,
    .upgrades_revealed = {},
    .upgrades_required = {UPGRADEID_Unlock_Arcana},
    .abilities_unlocked = {ABILITYID_Fire_Bolt},
    .skills_unlocked = {},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .name = "Learn Fire Bolt",
    .description = "",
    .level = 0,
};

Upgrade multishot =  
{
    .upgrade_ID = UPGRADEID_Multishot,
    .upgrades_revealed = {},
    .upgrades_required = {UPGRADEID_Unlock_Fire_Bolt, UPGRADEID_Unlock_Arcana},
    .abilities_unlocked = {},
    .skills_unlocked = {},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .name = "Multishot",
    .description = "",
    .level = 0,
};

Upgrade upgrades[UPGRADEID_MAX];

void load_upgrade_data() 
{
    upgrades[UPGRADEID_Unlock_Mana] = unlock_mana;
    upgrades[UPGRADEID_Unlock_Magic] = unlock_magic;
    upgrades[UPGRADEID_Unlock_Arcana] = unlock_arcana;
    upgrades[UPGRADEID_Unlock_Fire_Bolt] = unlock_firebolt;
    upgrades[UPGRADEID_Multishot] = multishot;
}

Upgrade known_upgrades[UPGRADEID_MAX];

bool is_upgrade_in_known(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (known_upgrades[i].upgrade_ID == upgrade_ID)
        {
            return true; // Upgrade already exists
        }
    }
    return false; // Upgrade not found
}

void update_known_upgrades()
{
    int known_count = 0;

    // Clear unlocked upgrades
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (known_upgrades[i].unlocked == true)
        {
            known_upgrades[i].known = false;
        }
    }

    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (upgrades[i].known && !upgrades[i].unlocked)
        {
            // Check if upgrade is already in known
            if (!is_upgrade_in_known(upgrades[i].upgrade_ID))
            {
                known_upgrades[known_count] = upgrades[i];
                known_count++;
            }
        }
    }
}

void mark_upgrade_unlocked(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (upgrades[i].upgrade_ID == upgrade_ID)
        {
            upgrades[i].unlocked = true;
            //log("Upgrade '%s' unlocked!\n", upgrades[i].name);
            return;
        }
    }
    log("Upgrade with ID %i not found.\n", upgrade_ID);
}