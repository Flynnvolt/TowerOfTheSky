#pragma once

typedef enum UpgradeID UpgradeID;

enum UpgradeID
{
    UPGRADE_Multishot,
};

typedef struct Upgrade Upgrade;

struct Upgrade
{
    UpgradeID upgrade_ID;
    bool unlocked;
    bool known;
    int level;
};

Upgrade multishot = 
{
    .upgrade_ID = UPGRADE_Multishot,
    .unlocked = false,
    .known = false,
    .level = 0,
};