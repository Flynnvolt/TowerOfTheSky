#pragma once
#include "Sprites.c"

typedef enum ItemID ItemID;

enum ItemID
{
	ITEM_Nil,
	ITEM_Rock,
	ITEM_Pine_Wood,
	ITEM_Exp,
	ITEM_MAX,
};

typedef struct ItemData ItemData;

struct ItemData
{
	SpriteData sprite_data;
	bool is_valid;
	ItemID item_ID;
    char pretty_name[32];
    char description[128];
	int amount;
};