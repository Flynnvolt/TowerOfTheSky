#pragma once
#include "Limits.c"
#include "Entities.c"
#include "Building.c"

typedef Vector2i Tile;
 
typedef struct TileData TileData;

struct TileData 
{
	Tile tile;
	bool is_valid;
	BuildingData building;
};

typedef struct FloorData FloorData;

struct FloorData
{
	int floor_ID;
	bool is_valid;
	TileData tiles[MAX_TILE_COUNT];
	Entity entities[MAX_ENTITY_COUNT];
};