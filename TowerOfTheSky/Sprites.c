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

Vector2 get_sprite_size(SpriteData sprite_data)
{
    if (sprite_data.image == NULL) 
    {
        log("Error: spritedata or image is NULL.\n");
        return (Vector2) {0, 0};
    }

    return (Vector2) {sprite_data.image -> width, sprite_data.image -> height};
}

void load_sprite_data()
{
	// :Load Sprites

	// Missing Texture Sprite
	sprites[0] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/missing_tex.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Nil
	};

	// Player
	sprites[SPRITE_Player] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/player.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Player
	};

	// Entities
	sprites[SPRITE_Target] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/Target.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Target
	};

	sprites[SPRITE_Slime] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/slime.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Slime
	};

	// Items
	sprites[SPRITE_Exp] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Exp
	};

	// Buildings
	sprites[SPRITE_Wall] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/wall.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Wall
	};

	sprites[SPRITE_Research_Station] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/research_station.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Research_Station
	};

	sprites[SPRITE_Exp_Vein] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp_vein.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Exp_Vein
	};

	sprites[SPRITE_Stairs_Up] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/stairs_up.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Stairs_Up
	};

	sprites[SPRITE_Stairs_Down] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/stairs_down.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Stairs_Down
	};

	sprites[SPRITE_Crate] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/crate.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Crate
	};

	// Spells
	sprites[SPRITE_Fireball_Sheet] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/fireball_sprite_sheet.png"), get_heap_allocator()), 
		.sprite_ID = SPRITE_Fireball_Sheet
	};

	#if CONFIGURATION == DEBUG
		{
			for (SpriteID i = 0; i < SPRITE_MAX; i++) 
			{
				SpriteData* sprite = & sprites[i];
				assert(sprite -> image, "Sprite was not setup properly");
			}
		}
	#endif
}

Gfx_Image* get_image_by_id(SpriteID sprite_ID) 
{
    return sprites[sprite_ID].image;
}