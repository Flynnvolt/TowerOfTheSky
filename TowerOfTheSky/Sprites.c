#pragma once

string char_to_string(char* char_array) 
{
    return STR(char_array); 
}

void get_executable_path(char* out_path, size_t max_length)
{
    // Get the full path of the executable
    GetModuleFileName(NULL, out_path, (DWORD)max_length);

    // Find the last backslash to remove the executable name and keep only the directory
    for (size_t i = strlen(out_path); i > 0; --i)
    {
        if (out_path[i] == '\\')
        {
            out_path[i] = '\0';  // Terminate the string after the last backslash
            break;
        }
    }
}

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

void load_sprite_data(const string base_path)
{
    string full_path;

    // Missing Texture Sprite
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\missing_tex.png"), get_temporary_allocator());
    sprites[0] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Nil
    };

    // Player
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\player.png"), get_temporary_allocator());
    sprites[SPRITE_Player] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Player
    };

    // Entities
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\Target.png"), get_temporary_allocator());
    sprites[SPRITE_Target] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Target
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\slime.png"), get_temporary_allocator());
    sprites[SPRITE_Slime] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Slime
    };

    // Items
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\exp.png"), get_temporary_allocator());
    sprites[SPRITE_Exp] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Exp
    };

    // Buildings
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\wall.png"), get_temporary_allocator());
    sprites[SPRITE_Wall] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Wall
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\research_station.png"), get_temporary_allocator());
    sprites[SPRITE_Research_Station] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Research_Station
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\exp_vein.png"), get_temporary_allocator());
    sprites[SPRITE_Exp_Vein] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Exp_Vein
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\stairs_up.png"), get_temporary_allocator());
    sprites[SPRITE_Stairs_Up] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Stairs_Up
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\stairs_down.png"), get_temporary_allocator());
    sprites[SPRITE_Stairs_Down] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Stairs_Down
    };

    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\crate.png"), get_temporary_allocator());
    sprites[SPRITE_Crate] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Crate
    };

    // Spells
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\fireball_sprite_sheet.png"), get_temporary_allocator());
    sprites[SPRITE_Fireball_Sheet] = (SpriteData)
    { 
        .image = load_image_from_disk(full_path, get_heap_allocator()), 
        .sprite_ID = SPRITE_Fireball_Sheet
    };

    #if CONFIGURATION == DEBUG
        for (SpriteID i = 0; i < SPRITE_MAX; i++) 
        {
            SpriteData* sprite = &sprites[i];
            assert(sprite->image, "Sprite was not setup properly");
        }
    #endif
}

Gfx_Image* get_image_by_id(SpriteID sprite_ID) 
{
    return sprites[sprite_ID].image;
}