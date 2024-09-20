#pragma once

typedef enum SpriteID SpriteID;

enum SpriteID
{
	SPRITE_Nil = 0,
	SPRITE_Player = 1,
	SPRITE_Research_Station = 2,
	SPRITE_Exp = 3,
	SPRITE_Exp_Vein = 4,
	SPRITE_Fireball_Sheet = 5,
	SPRITE_Target = 6,
	SPRITE_Wall = 7,
	SPRITE_Stairs_Up = 8,
	SPRITE_Stairs_Down = 9,
	SPRITE_Crate = 10,
	SPRITE_Slime = 11,
	SPRITE_MAX,
};

typedef struct SpriteData SpriteData;

struct SpriteData
{
	Gfx_Image* image;
	SpriteID sprite_ID;
};

SpriteData sprites[SPRITE_MAX];