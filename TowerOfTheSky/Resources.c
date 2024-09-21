#pragma once

typedef enum ResourceID ResourceID;

enum ResourceID
{
    RESOURCEID_Mana,
    RESOURCEID_Intellect,
    RESOURCEID_MAX,
};

typedef struct Resource Resource;

struct Resource 
{
    ResourceID resource_ID;
    bool unlocked;
    char name[32];
    float current;
    float max;
    float per_second;
};

// Resources

Resource mana = 
{
    .resource_ID = RESOURCEID_Mana,
    .unlocked = true,
    .name = "Mana",
    .current = 0.0,
    .max = 100.0,
    .per_second = 10.0,
};

Resource intellect = 
{
    .resource_ID = RESOURCEID_Intellect,
    .unlocked = false,
    .name = "Intellect",
    .current = 0.0, 
    .max = 50.0,
    .per_second = 0.5,
};