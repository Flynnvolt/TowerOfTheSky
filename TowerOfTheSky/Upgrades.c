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
    AbilityUpgrade ability_upgrade;
    UpgradeID upgrades_required[UPGRADEID_MAX];
    UpgradeID upgrades_revealed[UPGRADEID_MAX];
    AbilityID abilities_unlocked[ABILITYID_MAX];
    AbilityID abilities_upgraded[ABILITYID_MAX];
    SkillID skills_unlocked[SKILLID_MAX];
    ResourceID resources_unlocked[RESOURCEID_MAX];
    bool known;
    bool unlocked;
    bool has_levels;
    char name[32];
    char description[128];
    char level_up_text[128];
    int level;
};

Upgrade unlock_mana = 
{
    .upgrade_ID = UPGRADEID_Unlock_Mana,
    .ability_upgrade = {},
    .upgrades_revealed = {UPGRADEID_Unlock_Magic},
    .abilities_unlocked = {},
    .abilities_upgraded = {},
    .upgrades_required = {},
    .skills_unlocked = {SKILLID_Channel_Mana},
    .resources_unlocked = {RESOURCEID_Mana},
    .known = true,
    .unlocked = false,
    .has_levels = false,
    .name = "Study Mana",
    .description = "Extensive studying of mana\n allows you replicate it for yourself.",
    .level_up_text = "",
    .level = 0,
};

Upgrade unlock_magic = 
{
    .upgrade_ID = UPGRADEID_Unlock_Magic,
    .ability_upgrade = {},
    .upgrades_revealed = {UPGRADEID_Unlock_Arcana, UPGRADEID_Unlock_Fire_Bolt},
    .upgrades_required = {UPGRADEID_Unlock_Mana},
    .abilities_unlocked = {},
    .abilities_upgraded = {},
    .skills_unlocked = {SKILLID_Wisdom},
    .resources_unlocked = {RESOURCEID_Intellect},
    .known = false,
    .unlocked = false,
    .has_levels = false,
    .name = "Study Magic",
    .description = "You delve further into your studies\n of magic and discover\n even more power.",
    .level_up_text = "",
    .level = 0,
};

Upgrade unlock_arcana = 
{
    .upgrade_ID = UPGRADEID_Unlock_Arcana,
    .ability_upgrade = {},
    .upgrades_revealed = {UPGRADEID_Multishot},
    .upgrades_required = {UPGRADEID_Unlock_Magic},
    .abilities_unlocked = {},
    .abilities_upgraded = {},
    .skills_unlocked = {SKILLID_Focus},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .has_levels = false,
    .name = "Study Arcana",
    .description = "After great effort\n you are able to cast more\n advanced magic.",
    .level_up_text = "",
    .level = 0,
};

Upgrade unlock_firebolt = 
{
    .upgrade_ID = UPGRADEID_Unlock_Fire_Bolt,
    .ability_upgrade = {},
    .upgrades_revealed = {},
    .upgrades_required = {UPGRADEID_Unlock_Arcana},
    .abilities_unlocked = {ABILITYID_Fire_Bolt}, // make sure only main skill has this filled.
    .abilities_upgraded = {ABILITYID_Fire_Bolt}, // make sure both the main skill and upgrades to main skills have this filled.
    .skills_unlocked = {},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .has_levels = true,
    .name = "Learn Fire Bolt",
    .description = "A spell of fire,\n a simple yet powerful magic.",
    .level_up_text = "Firebolt",
    .level = 0,
};

Upgrade multishot =  
{
    .upgrade_ID = UPGRADEID_Multishot,
    .ability_upgrade = ABILITYUPGRADEID_Multishot,
    .upgrades_revealed = {},
    .upgrades_required = {UPGRADEID_Unlock_Fire_Bolt, UPGRADEID_Unlock_Arcana},
    .abilities_unlocked = {},
    .abilities_upgraded = {ABILITYID_Fire_Bolt},
    .skills_unlocked = {},
    .resources_unlocked = {},
    .known = false,
    .unlocked = false,
    .has_levels = true,
    .name = "Learn Multishot",
    .description = "Muticasting is very powerful,\n at a cost.",
    .level_up_text = "Multishot",
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