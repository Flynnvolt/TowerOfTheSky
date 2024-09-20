#pragma once
#include "Sprites.c"

typedef enum BuildingID BuildingID;

enum BuildingID 
{
	BUILDING_Nil,
	BUILDING_Stairs_Up,
	BUILDING_stairs_Down,
	BUILDING_Research_Station,
	BUILDING_Crate,
	BUILDING_Wall,
	BUILDING_MAX,
};

typedef struct BuildingData BuildingData;

struct BuildingData 
{
	BuildingID building_ID;
	SpriteData sprite_data;
	bool is_valid;
	Vector2 pos;
    char pretty_name[32];
    char description[128];
	int current_floor;
};