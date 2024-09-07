#include "TowerOfTheSky.h"

// Defines

#define DEV_TESTING

#define m4_identity m4_make_scale(v3(1, 1, 1))

#define MAX_PROJECTILES 100 

#define MAX_ENTITY_COUNT 1024

#define MAX_TILE_COUNT 4096

#define MAX_FLOOR_COUNT 10

// :Global APP

float64 delta_t;

Gfx_Font* font;

u32 font_height = 48;

float screen_width = 480.0;

float screen_height = 270.0;

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

// :Variables

Vector4 bg_box_color = {0, 0, 0, 0.5};

Vector4 color_0;

// :Sprites

typedef enum SpriteID SpriteID;

enum SpriteID
{
	SPRITE_nil = 0,
	SPRITE_player = 1,
	SPRITE_research_station = 2,
	SPRITE_exp = 3,
	SPRITE_exp_vein = 4,
	SPRITE_fireball_sheet = 5,
	SPRITE_target = 6,
	SPRITE_stairs = 7,
	SPRITE_MAX,
};

typedef struct SpriteData SpriteData;

struct SpriteData
{
	Gfx_Image* image;
	SpriteID spriteID;
};

SpriteData sprites[SPRITE_MAX];

// :Items

typedef enum ItemID ItemID;

enum ItemID
{
	ITEM_nil,
	ITEM_rock,
	ITEM_pine_wood,
	ITEM_exp,
	ITEM_MAX,
};

typedef struct ItemData ItemData;

struct ItemData
{
	SpriteData spriteData;
	bool is_valid;
	ItemID itemID;
    char pretty_name[32];
    char description[128];
	int amount;
};

// :Entities

typedef enum EntityID EntityID;

enum EntityID
{
	ENTITY_nil,
	ENTITY_player,
	ENTITY_enemy,
	ENTITY_MAX,
};

typedef struct Entity Entity;

struct Entity
{
	EntityID entityID;
	bool is_valid;
	SpriteID spriteID;
	Vector2 pos;
	float health;
	float max_health;
	float health_regen;
	bool is_immortal;
	float xRemainder;
	float yRemainder;
	int current_floor;
};

typedef enum BuildingID BuildingID;

enum BuildingID 
{
	BUILDING_nil,
	BUILDING_stairs,
	BUILDING_research_station,
	BUILDING_MAX,
};

typedef struct BuildingData BuildingData;

struct BuildingData 
{
	BuildingID buildingID;
	SpriteData spriteData;
	bool is_valid;
    char pretty_name[32];
    char description[128];
};

// :Projectiles

typedef struct Projectile Projectile;

struct Projectile 
{
    bool is_active;

    AnimationInfo animation;  

    Vector2 position;
    Vector2 velocity;

    Vector2 target_position; 

    float distance_traveled;
    float max_distance;
	float time_alive;
	float max_time_alive;

    float speed;              
    float rotation;
    float damage;
    float scale;
    float radius;
	float xRemainder;
	float yRemainder;

	Entity *source_entity; // Caster of the Projectile
};

// :UX

typedef enum UXState UXState;

enum UXState
{
	UX_nil,
	UX_inventory,
	UX_research,
};

typedef Vector2i Tile;
 
typedef struct TileData TileData;

struct TileData 
{
	Tile tile;
	BuildingData building;
};

typedef struct FloorData FloorData;

struct FloorData
{
	int floorID;
	bool is_valid;
	TileData tiles[MAX_TILE_COUNT];
	Entity entities[MAX_ENTITY_COUNT];
	Projectile projectiles[MAX_PROJECTILES];
};

// :World

typedef struct World World;

struct World
{
	float64 time_elapsed;

	UXState ux_state;

	FloorData floors[MAX_FLOOR_COUNT];

	ItemData items[ITEM_MAX];

	int current_floor;

	float inventory_alpha;

	float inventory_alpha_target;

	float building_alpha;

	float building_alpha_target;
};

World* world = 0;

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	Matrix4 world_proj;
	Matrix4 world_view;
	bool hover_consumed;
	Entity* player;
};

WorldFrame world_frame;

// :Setup

Vector2 get_sprite_size(SpriteData spritedata)
{
    if (spritedata.image == NULL) 
    {
        log("Error: spritedata or image is NULL.\n");
        return (Vector2) {0, 0}; // Return a default value or handle the error as needed
    }

    return (Vector2) {spritedata.image -> width, spritedata.image -> height};
}

void LoadSpriteData()
{
	// :Load Sprites

	// Missing Texture Sprite
	sprites[0] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/missing_tex.png"), get_heap_allocator()), 
		.spriteID = SPRITE_nil
	};

	// Player
	sprites[SPRITE_player] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/player.png"), get_heap_allocator()), 
		.spriteID = SPRITE_player
	};

	// Entities
	sprites[SPRITE_target] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/Target.png"), get_heap_allocator()), 
		.spriteID = SPRITE_target
	};

	// Items
	sprites[SPRITE_exp] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp.png"), get_heap_allocator()), 
		.spriteID = SPRITE_exp
	};

	// Buildings
	sprites[SPRITE_research_station] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/research_station.png"), get_heap_allocator()), 
		.spriteID = SPRITE_research_station
	};

	sprites[SPRITE_exp_vein] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp_vein.png"), get_heap_allocator()), 
		.spriteID = SPRITE_exp_vein
	};

	sprites[SPRITE_stairs] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/stairs.png"), get_heap_allocator()), 
		.spriteID = SPRITE_stairs
	};

	// Spells
	sprites[SPRITE_fireball_sheet] = (SpriteData)
	{ 
		.image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/fireball_sprite_sheet.png"), get_heap_allocator()), 
		.spriteID = SPRITE_fireball_sheet
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

Entity* get_player() 
{
	return world_frame.player;
}

void setup_player(Entity* player_en) 
{
	player_en -> entityID = ENTITY_player;
    player_en -> spriteID = SPRITE_player;
	player_en -> health = 100;
	player_en -> max_health = 100;
	player_en -> health_regen = 4;
	player_en -> pos = v2(0, 0);
	player_en -> pos = round_v2_to_tile(player_en -> pos);
	player_en -> pos.y -= tile_width * 0.5;
	player_en -> pos.x -= sprites[player_en -> spriteID].image -> width * 0.5;

	// Center X and Y
	//player_en -> pos = v2((player_en -> pos.x - sprites[player_en -> spriteID].image -> width * 0.5),(player_en -> pos.y - sprites[player_en -> spriteID].image -> height * 0.5));
}

void setup_target(Entity* en) 
{
	en -> entityID = ENTITY_enemy;
    en -> spriteID = SPRITE_target;
	en -> health = 100;
	en -> max_health = 100;
	en -> health_regen = 25;
	en -> is_immortal = true;
	en -> pos = v2(0, 100);
	en -> pos = round_v2_to_tile(en -> pos);
	en -> pos.y -= tile_width * 0.5;
	en -> pos.x -= sprites[en -> spriteID].image -> width * 0.5;
}

ItemData setup_exp_item() 
{
    ItemData item;
    item.itemID = ITEM_exp;
    item.spriteData = sprites[SPRITE_exp];
    item.amount = 50;
    item.is_valid = true;
    strcpy(item.pretty_name, "EXP"); 
    strcpy(item.description, "Experience points for leveling up"); 
    return item;
}

BuildingData setup_building_stairs() 
{
    BuildingData building;
    building.buildingID = BUILDING_stairs;
    building.spriteData = sprites[SPRITE_stairs];
    building.is_valid = true;
    strcpy(building.pretty_name, "Stairs"); 
    strcpy(building.description, "Travel between floors"); 
    return  building;
}

Entity* entity_create() 
{
	Entity* entity_found = 0;

	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity* existing_entity = & world -> floors[world -> current_floor].entities[i];

		if (!existing_entity -> is_valid) 
		{
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "No more free entities!");

	entity_found -> is_valid = true;
	entity_found -> current_floor = world -> current_floor;

	return entity_found;
}

void entity_destroy(Entity* entity) 
{
	memset(entity, 0, sizeof(Entity));
}

// :Functions

void create_circle_floor_data(FloorData* floor, float tile_radius, int tile_width)
{
    float tile_radius_squared = tile_radius * tile_radius;
    
    int tile_count = 0;
    for (int x = -(int)tile_radius; x <= (int)tile_radius; x++) 
    {
        for (int y = -(int)tile_radius; y <= (int)tile_radius; y++) 
        {
            float dx = (float)x;
            float dy = (float)y;
            float distance_squared = dx * dx + dy * dy;

            if (distance_squared < tile_radius_squared && tile_count < MAX_TILE_COUNT) 
            {

                TileData* tile_data = & floor -> tiles[tile_count];
                tile_data -> tile.x = x;
                tile_data -> tile.y = y;

                // zero building data for now
                memset(& tile_data -> building, 0, sizeof(BuildingData));

                tile_count++;
            }
        }
    }
    /*
    // initialize remaining unused tiles
    for (int i = tile_count; i < MAX_TILE_COUNT; i++) 
    {
        memset(& floor -> tiles[i], 0, sizeof(TileData));
    }
	*/
	//log("%i", tile_count);
}

FloorData create_empty_floor()
{
	FloorData floor;
	memset(& floor, 0, sizeof(FloorData)); // Initialize the FloorData structure

	create_circle_floor_data(& floor, tile_radius, tile_width);

	return floor;
}

void render_floor_tiles(FloorData* floor, float tile_width, Vector4 color_0)
{	
    float half_tile_width = tile_width * 0.5f;

    for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
        TileData* tile_data = & floor -> tiles[i];
        
        // Get the tile's x and y position
        int x = tile_data -> tile.x;
       	int y = tile_data -> tile.y;

        // Checkerboard pattern
        Vector4 col = color_0;
        if (((x + y) % 2) == 0) 
        {
            col.a = 0.75;
        }
        
        // Calculate world position for the tile
        float x_pos = x * tile_width;
        float y_pos = y * tile_width;
        
        // Render tile
        draw_rect(v2(x_pos - half_tile_width, y_pos - half_tile_width), v2(tile_width, tile_width), col);
    }
	// Debug:
	/*
	Vector2 mouse_pos_world = get_mouse_pos_in_world_space();
	int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
	int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);
	
	// Display world space position of tile and tile X, Y
	draw_rect(v2(tile_pos_to_world_pos(mouse_tile_x) - half_tile_width, tile_pos_to_world_pos(mouse_tile_y) - half_tile_width), v2(tile_width, tile_width), v4(0.5, 0.0, 0.0, 1.0));
	draw_text(font, sprint(get_temporary_allocator(), STR("%.1f %.1f (%i, %i)"), (tile_pos_to_world_pos(mouse_tile_x)), (tile_pos_to_world_pos(mouse_tile_y)), world_pos_to_tile_pos(tile_pos_to_world_pos(mouse_tile_x)), world_pos_to_tile_pos(tile_pos_to_world_pos(mouse_tile_y))), font_height, v2((tile_pos_to_world_pos(mouse_tile_x) - half_tile_width), (tile_pos_to_world_pos(mouse_tile_y) - half_tile_width)), v2(0.2, 0.2), COLOR_WHITE);
	*/
}

void load_next_floor()
{
	if(world -> floors[world -> current_floor + 1].is_valid == true)
	{
		world -> current_floor++;

	}
	else
	{
		world -> floors[world -> current_floor + 1] = create_empty_floor();
		world -> floors[world -> current_floor + 1].is_valid = true;
		world -> floors[world -> current_floor + 1].floorID = world -> current_floor + 1;
		world -> current_floor++;
	}
}

void load_previous_floor()
{
	if(world -> floors[world -> current_floor - 1].is_valid == true)
	{
		world -> current_floor--;
	}
	else
	{
		world -> floors[world -> current_floor - 1] = create_empty_floor();
		world -> floors[world -> current_floor - 1].is_valid = true;
		world -> floors[world -> current_floor - 1].floorID = world -> current_floor - 1;
		world -> current_floor--;
	}
}

void render_buildings(FloorData* floor, float tile_width, Vector4 color_0)
{
    float half_tile_width = tile_width * 0.5f;

    for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
        TileData* tile_data = & floor -> tiles[i];
        
        // Get the tile's x and y position
        int x = tile_data -> tile.x;
        int y = tile_data -> tile.y;

		// Calculate world position for the tile
        float x_pos = x * tile_width;
        float y_pos = y * tile_width;

		if(tile_data -> building.buildingID != 0)
		{
			SpriteData spritedata = tile_data -> building.spriteData;

			Vector2 sprite_size = get_sprite_size(spritedata);

			Matrix4 xform = m4_scalar(1.0);

			xform = m4_translate(xform, v3(x_pos - half_tile_width, y_pos - half_tile_width, 0));

			draw_image_xform(sprites[tile_data -> building.spriteData.spriteID].image, xform, sprite_size, COLOR_WHITE);
		}
    }
}

inline float64 now() 
{
	return world -> time_elapsed;
}

float alpha_from_end_time(float64 end_time, float length) 
{
	return float_alpha(now(), end_time-length, end_time);
}

bool has_reached_end_time(float64 end_time) 
{
	return now() > end_time;
}

Gfx_Image* get_image_by_id(SpriteID spriteID) 
{
    return sprites[spriteID].image;
}

SpriteData* get_sprite(SpriteID id)
{
	if (id >= 0 && id < SPRITE_MAX)
	{
		SpriteData* sprite = & sprites[id];

		if (sprite -> image)
		{
			return sprite;
		}
		else
		{
			return & sprites[0];
		}
	}
	return & sprites[0];
}

void PlayerDeath() 
{

}

void DamagePlayer(Entity *entity, float damage)
{
	// We can do fancy player only stuff here if we want to.

	entity -> health -= damage;

	if (entity -> health <= 0)
	{
		PlayerDeath();
	}
}

void DamageEnemy(Entity *entity, float damage)
{	
	// We can do fancy enemy only stuff here if we want to.

	entity -> health -= damage;

	if (entity -> health <= 0)
	{
		if (entity -> is_immortal == true)
		{
			entity -> health = entity -> max_health;
		}
		else
		{
			entity -> is_valid = false;
		}
	}
}

void DamageEntity(Entity *entity, float damage)
{
	EntityID entityID = entity -> entityID;

	switch (entityID) 
	{
		case ENTITY_player:
		{
			DamagePlayer(entity, damage);
			break;
		}

		case ENTITY_enemy:
		{
			DamageEnemy(entity, damage);
			break;
		}
		
		default: 
		{
			log_error("misconfigured arch in Damage Entity"); 
			break;
		}
	}
}

bool collideAt(Entity *current_entity, int x, int y) 
{
    SpriteData spritedata = sprites[current_entity -> spriteID];
    int width1 = spritedata.image -> width;
    int height1 = spritedata.image -> height;

    int x_end1 = x + width1;
    int y_end1 = y + height1;

    for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
    {
        Entity *actor = & world -> floors[world -> current_floor].entities[i];

        // Skip ourselves to avoid self-collision
        if (actor -> is_valid && actor != current_entity) 
        {
            SpriteData spritedata2 = sprites[actor -> spriteID];
            int width2 = spritedata2.image -> width;
            int height2 = spritedata2.image -> height;

            int actor_x_end = actor -> pos.x + width2;
            int actor_y_end = actor-> pos.y + height2;

            // Check for bounding box overlap
            if (x < actor_x_end && x_end1 > actor -> pos.x &&
                y < actor_y_end && y_end1 > actor -> pos.y) 
            {
                //log("Collision detected with entity %d\n", i);
                return true;
            }
        }
    }

    // No collisions detected
    return false;
}

void collide_visual_debug(Entity *current_entity)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity *actor = & world -> floors[world -> current_floor].entities[i];

		if (actor -> is_valid && actor != current_entity) 
		{
			SpriteData spritedata = sprites[actor -> spriteID];
			int sprite_width = spritedata.image -> width;
			int sprite_height = spritedata.image -> height;

			SpriteData spritedata2 = sprites[current_entity -> spriteID];

			// Visual Debug tools
			//draw_rect(v2(actor -> pos.x, actor -> pos.y), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2));  // Draw bounding box
			//draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(spritedata2.image -> width, spritedata2.image -> height), v4(255, 0, 0, 0.2));  // Draw bounding box
			//draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(1, 1), v4(0, 255, 255, 1)); // Where we are
		}
	}
}

void MoveEntityX(Entity *entity, float amount) 
{
    entity -> xRemainder += amount;
    int move = roundf(entity -> xRemainder);

    if (move != 0) 
    {
        entity -> xRemainder -= move;
        int movement_direction = (move > 0) - (move < 0);

        // Check collision with the bounding box after moving
        if (!collideAt(entity, entity -> pos.x + movement_direction, entity -> pos.y)) 
        {
            entity -> pos.x += movement_direction;
            move -= movement_direction;
        }
    }
}

void MoveEntityY(Entity *entity, float amount) 
{
    entity -> yRemainder += amount;
    int move = roundf(entity -> yRemainder);

    if (move != 0) 
    {
        entity -> yRemainder -= move;
        int movement_direction = (move > 0) - (move < 0);

        // Check collision with the bounding box after moving
        if (!collideAt(entity, entity -> pos.x, entity -> pos.y + movement_direction)) 
        {
            entity -> pos.y += movement_direction;
            move -= movement_direction;
        }
    }
}

void updateEntity(Entity *entity, Vector2 movement) 
{
	MoveEntityX(entity, movement.x);

	MoveEntityY(entity, movement.y);

	collide_visual_debug(entity);
}

typedef struct DebugCircleState DebugCircleState;

struct DebugCircleState
{
    bool active;
    Vector2 center;
    float radius;
    float time_remaining;
};

DebugCircleState circle_state = {0};

void start_debug_circle(DebugCircleState *state, Vector2 center, float radius, float duration) 
{
    state -> active = true;
    state -> center = center;
    state -> radius = radius;
    state -> time_remaining = duration;
}

void update_debug_circle(DebugCircleState *state) 
{
    if (state -> active) 
    {
        if (state -> time_remaining > 0.0f) 
        {
            Vector2 circle_size = v2(state -> radius * 2.0f, state -> radius * 2.0f);
            Vector4 circle_color = v4(255, 0, 0, 1); // Red color

			// Center it on the position
            draw_circle(v2(state -> center.x - state -> radius, state -> center.y - state -> radius), circle_size, circle_color);

            // Draw the current position of the circle for debugging
            draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), state -> center.x, state -> center.y), font_height, state -> center, v2(0.2, 0.2), COLOR_WHITE);

            state -> time_remaining -= delta_t;

            if (state -> time_remaining <= 0.0f) 
            {
                state -> active = false;
            }
        }
    }
}

void spawn_projectile(Entity *source_entity, float speed, float damage, AnimationInfo *animation, float32 scale, float spawn_radius, float max_distance, float max_time_alive) 
{
    for (int i = 0; i < MAX_PROJECTILES; i++) 
    {
        if (!world -> floors -> projectiles[i].is_active) 
        {
            Projectile *projectile = & world -> floors -> projectiles[i];
            projectile -> is_active = true;
            projectile -> speed = speed;
            projectile -> damage = damage;
            projectile -> animation = *animation;
            projectile -> scale = scale;
            projectile -> source_entity = source_entity;
            projectile -> distance_traveled = 0.0f;
            projectile -> max_distance = max_distance;
            projectile -> time_alive = 0.0f;
            projectile -> max_time_alive = max_time_alive;

            // Calculate the player's center position
            SpriteData spritedata = sprites[source_entity -> spriteID];
            Vector2 player_center = v2((source_entity -> pos.x + (spritedata.image -> width * 0.5f)), 
                                       (source_entity -> pos.y + (spritedata.image -> height * 0.5f)));

            Vector2 mouse_pos = get_mouse_pos_in_world_space();

            // Calculate direction from player to mouse
            Vector2 direction = v2_sub(mouse_pos, player_center);
            float32 length = v2_length(direction);

            // Normalize the direction vector (to get the direction to shoot in)
            Vector2 normalized_direction = direction;
            if (length != 0.0f)
            {
                normalized_direction = v2_scale(direction, 1.0f / length);
            }

            // Calculate the angle based on the direction from player center to mouse
            float angle = atan2f(normalized_direction.y, normalized_direction.x);

            // Spawn the projectile at the edge of the spawn_radius circle, based on the calculated angle
            Vector2 spawn_position = v2_add(player_center, v2(spawn_radius * cosf(angle), spawn_radius * sinf(angle)));

            projectile -> position = spawn_position;

            // Debug logging
            // printf("Player Center: (%f, %f)\n", player_center.x, player_center.y);
            // printf("Spawn Position: (%f, %f)\n", spawn_position.x, spawn_position.y);
            // printf("Mouse Position: (%f, %f)\n", mouse_pos.x, mouse_pos.y);
            // printf("Projectile Direction Angle: %f\n", angle);
            // printf("Spawn Radius: %f\n", spawn_radius);

            // Draw the debug circle around the player when projectile is cast
            start_debug_circle(& circle_state, player_center, spawn_radius, 1.0);

            // Use the actual direction to the mouse (not the spawn position) for velocity calculation
            if (length != 0.0f)
            {
                Vector2 velocity_direction = normalized_direction;  // Already normalized direction
                projectile -> velocity = v2_scale(velocity_direction, speed);

                // Set rotation based on the direction the projectile is moving
                projectile -> rotation = atan2f(-velocity_direction.y, velocity_direction.x) * (180.0f / PI32);

                if (projectile -> rotation < 0.0f)
                {
                    projectile -> rotation += 360.0f;
                }
            }
            break;
        }
    }
}

Entity* projectile_collides_with_entity(Projectile *projectile) 
{
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
    {
        Entity *entity = & world -> floors -> entities[i];

        if (entity -> is_valid && entity != projectile -> source_entity) // Skip the source entity
        {
            SpriteData spritedata = sprites[entity -> spriteID];
            int entity_width = spritedata.image -> width;
            int entity_height = spritedata.image -> height;

            int entity_x_end = entity -> pos.x + entity_width;
            int entity_y_end = entity -> pos.y + entity_height;

            int projectile_x_end = projectile -> position.x + projectile -> radius * 2;
            int projectile_y_end = projectile -> position.y + projectile -> radius * 2;

            if (projectile -> position.x < entity_x_end && projectile_x_end > entity -> pos.x && projectile -> position.y < entity_y_end && projectile_y_end > entity -> pos.y) 
            {
                return entity;
            }
        }
    }
    return NULL;
}

void update_projectile(Projectile *projectile, float delta_time) 
{
    if (!projectile -> is_active) return;

    // Update the time alive
    projectile -> time_alive += delta_time;

    // Update position based on velocity scaled by delta time
    Vector2 movement = v2_scale(projectile -> velocity, delta_time);

    // Accumulate the movement in the remainder variables
    projectile -> xRemainder += movement.x;
    projectile -> yRemainder += movement.y;

    // Calculate how much to move this frame (rounded to the nearest pixel)
    int moveX = roundf(projectile -> xRemainder);
    int moveY = roundf(projectile -> yRemainder);

    // Update the position and reset the remainders
    if (moveX != 0) 
    {
        projectile -> position.x += moveX;
        projectile -> xRemainder -= moveX;
    }

    if (moveY != 0) 
    {
        projectile -> position.y += moveY;
        projectile -> yRemainder -= moveY;
    }

    // Update the distance traveled
    projectile -> distance_traveled += v2_length(movement);

    // Check if the projectile hits any entity
    Entity *hit_entity = projectile_collides_with_entity(projectile);
    if (hit_entity) 
    {
        DamageEntity(hit_entity, projectile -> damage);

        // Deactivate the projectile after it hits an entity
        projectile -> is_active = false;
        return;
    }

    // Check if the projectile should expire
    if (projectile -> distance_traveled >= projectile->max_distance || projectile -> time_alive >= projectile -> max_time_alive) 
    {
        projectile -> is_active = false;
        return;
    }

    update_animation(& projectile -> animation, & projectile -> position, projectile -> scale, & projectile -> rotation);
}

Projectile fireball;

void draw_resource_bar(float y_pos, float *current_resource, float *max_resource, float *resource_per_second, int icon_size, int icon_row_count, Vector4 color, Vector4 bg_color, string *resource_name)
{
	// Increment resource
	if(*current_resource < *max_resource)
	{
		*current_resource += *resource_per_second * delta_t;
	}

	// Resource Overflow Check
	if(*current_resource >= *max_resource)
	{
		*current_resource = *max_resource;
	}

	// Resource underflow check
	if(*current_resource <= 0)
	{
		*current_resource = 0;
	}

	//log("%f %f %f", current_resource, max_resource, resource_per_second);

	float bar_width = icon_size * icon_row_count;

	float x_start_pos = (screen_width * 0.025);

	int current_resource_int = (int)*current_resource;

	int max_resource_int = (int)*max_resource;

	float percentage_of_bar_width = (bar_width / 100.0);

	float current_resource_percentage = (*current_resource / *max_resource) * 100.0f;

	float bar_visual_size = (percentage_of_bar_width * current_resource_percentage);

	// Black background box
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
		draw_rect_xform(xform, v2(bar_width, icon_size), bg_color);
	}

	// Bar Fill
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
		draw_rect_xform(xform, v2(bar_visual_size, icon_size), color);
	}	

	// Bar current resource display
	{
		string current_resource_string = STR("%s: %i/%i    +%.1f/s"); // %i is where the number goes.

		current_resource_string = sprint(get_temporary_allocator(), current_resource_string, *resource_name, current_resource_int, max_resource_int, *resource_per_second);

		Gfx_Text_Metrics metrics = measure_text(font, current_resource_string, font_height, v2(0.20, 0.20));

		Vector2 draw_pos = v2(x_start_pos + (bar_width * 0.5), y_pos + 14);

		draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
		
		draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

		draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

		draw_text(font, current_resource_string, font_height, draw_pos, v2(0.20, 0.20), COLOR_WHITE);
	}
}

void draw_unit_bar(Vector2 position, float *current_value, float *max_value, float *recovery_per_second, int icon_size, int icon_row_count, Vector4 color, Vector4 bg_color)
{
	// Update bar
	if(*current_value < *max_value)
	{
		*current_value += *recovery_per_second * delta_t;
	}

	// Value overflow check
	if(*current_value >= *max_value)
	{
		*current_value = *max_value;
	}

	// Value underflow check
	if(*current_value <= 0)
	{
		*current_value = 0;
	}

	//log("%f %f %f", current_value, max_value, recovery_per_second);

	float bar_width = icon_size * icon_row_count;

	int current_value_int = (int)*current_value;

	int max_value_int = (int)*max_value;

	float percentage_of_bar_width = (bar_width / 100.0);

	float current_fill_percentage = (*current_value / *max_value) * 100.0f;

	float bar_visual_size = (percentage_of_bar_width * current_fill_percentage);

	// Black background box
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3((position.x - (bar_width * 0.5)), (position.y + icon_size), 0.0));
		draw_rect_xform(xform, v2(bar_width, icon_size), bg_color);
	}

	// Bar Fill
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3((position.x - (bar_width * 0.5)), (position.y + icon_size), 0.0));
		draw_rect_xform(xform, v2(bar_visual_size, icon_size), color);
	}	
}

Draw_Quad* draw_level_up_button(string button_tooltip, float button_size, Vector2 button_position, Vector4 color)
{
	Vector2 button_size_v2 = v2(16.0, 16.0);

	Matrix4 xform = m4_scalar(1.0);

	xform = m4_translate(xform, v3(button_position.x, button_position.y, 0.0));

	Vector2 icon_positon = v2(button_position.x, button_position.y);
	
	// White transparent box to show item slot is filled.
	Draw_Quad* quad = draw_rect_xform(xform, v2(button_size, button_size), color);

	// Setup box for mouse collision
	Range2f btn_range = range2f_make_bottom_left(button_position, button_size_v2);

	// Draw Button Text
	Gfx_Text_Metrics metrics = measure_text(font, button_tooltip, font_height, v2(0.1, 0.1));
	Vector2 draw_pos = v2((button_position.x + (button_size_v2.x * 0.5)), (button_position.y + (button_size_v2.y * 0.5)));
	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

	draw_text(font, button_tooltip, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

	return quad;
}

bool check_if_mouse_clicked_button(Vector2 button_pos, Vector2 button_size)
{
	Range2f btn_range = range2f_make_bottom_left(button_pos, button_size);

	// Check if mouse is on button
	if (range2f_contains(btn_range, get_mouse_pos_in_world_space())) 
	{
		if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) 
		{
			consume_key_just_pressed(MOUSE_BUTTON_LEFT);

			// Button clicked
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool check_if_mouse_hovering_button(Vector2 button_pos, Vector2 button_size)
{
	Range2f btn_range = range2f_make_bottom_left(button_pos, button_size);

	// Check if mouse is on button
	if (range2f_contains(btn_range, get_mouse_pos_in_world_space())) 
	{
		return true;
	}
	else
	{
		return false;
	}
}

void draw_tooltip_box_string_below_same_size(Draw_Quad* quad, float tooltip_size, string *title)
{
	Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

	Range2f screen_range = quad_to_range(screen_quad);

	Vector2 icon_center = range2f_get_center(screen_range);

	Matrix4 xform = m4_scalar(1.0);

	Vector2 tooltip_box_size = v2(tooltip_size, tooltip_size);

	xform = m4_translate(xform, v3(tooltip_box_size.x * -0.5, - tooltip_box_size.y - tooltip_size * 0.5, 0));

	xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

	draw_rect_xform(xform, tooltip_box_size, bg_box_color);

	float current_y_pos = icon_center.y;

	// String Display	
	
	Gfx_Text_Metrics metrics = measure_text(font, *title, font_height, v2(0.1, 0.1));

	Vector2 draw_pos = icon_center;

	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	
	draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

	draw_pos = v2_add(draw_pos, v2(0, tooltip_size * -0.5));

	draw_pos = v2_add(draw_pos, v2(0, -2.0)); // Padding

	draw_text(font, *title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

	current_y_pos = draw_pos.y;
}

void draw_tooltip_box_string_to_side_larger_xform(Draw_Quad* quad, float tooltip_size, string *title)
{
	Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

	Range2f screen_range = quad_to_range(screen_quad);

	Vector2 icon_center = range2f_get_center(screen_range);

	Matrix4 xform = m4_scalar(1.0);

	Vector2 tooltip_box_size = v2(tooltip_size * 4, tooltip_size * 2);

	xform = m4_translate(xform, v3((tooltip_box_size.x) + (tooltip_size * 4), (- tooltip_box_size.y) + (tooltip_size * 0.5), 0));

	xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

	draw_rect_xform(xform, tooltip_box_size, bg_box_color);

	// String Display	

	Matrix4 text_xform = m4_scalar(1.0);

	text_xform = m4_translate(xform, v3((tooltip_box_size.x * 0.10), (tooltip_box_size.y * 0.65), 0));
	
	draw_text_xform(font, *title, font_height, text_xform, v2(0.1, 0.1), COLOR_WHITE);
}

void draw_tooltip_box_string_to_side_larger(Draw_Quad* quad, float tooltip_size, string *title)
{
	Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

	Range2f screen_range = quad_to_range(screen_quad);

	Vector2 icon_center = range2f_get_center(screen_range);

	Matrix4 xform = m4_scalar(1.0);

	Vector2 tooltip_box_size = v2(tooltip_size * 5, tooltip_size * 2);

	Vector2 button_position = v2(icon_center.x + (tooltip_box_size.x) + (tooltip_size * 5), icon_center.y + (- tooltip_box_size.y) + (tooltip_size * 0.5));

	xform = m4_translate(xform, v3(button_position.x, button_position.y, 0.0));
	
	// Draw tooltip background box
	Draw_Quad* tooltip_quad = draw_rect_xform(xform, tooltip_box_size, bg_box_color);

	// String Display	
	Gfx_Text_Metrics metrics = measure_text(font, *title, font_height, v2(0.1, 0.1));
	Vector2 draw_pos = v2((button_position.x + (tooltip_box_size.x * 0.5)), (button_position.y + (tooltip_box_size.y * 0.5)));
	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

	draw_text(font, *title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
}

// :UI Rendering

void set_screen_space()
{
	draw_frame.camera_xform = m4_scalar(1.0);
	draw_frame.projection = m4_make_orthographic_projection(0.0, screen_width, 0.0, screen_height, -1, 10);
}

void set_world_space() 
{
	draw_frame.projection = world_frame.world_proj;
	draw_frame.camera_xform = world_frame.world_view;
}

// :World init
void world_setup()
{
	//start inventory open
	world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);

	if(world -> current_floor == null)
	{
		world -> current_floor = 0;
	}

	world -> floors[world -> current_floor] = create_empty_floor();
	world -> floors[world -> current_floor].is_valid = true;
	world -> floors[world -> current_floor].floorID = world -> current_floor;

	// setup testing building
	for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
        TileData tile_data = world -> floors[world -> current_floor].tiles[i];
        
        // Get the tile's x and y position
        int x = tile_data.tile.x; 
        int y = tile_data.tile.y;

		//Place a staircase building at 5, 5
		if(x == 5 && y == 5)
		{
			world -> floors[world -> current_floor].tiles[i].building = setup_building_stairs();
		}
    }

	Entity* player_en = entity_create();
	setup_player(player_en);

	// :test stuff
	#if defined(DEV_TESTING)
	{
		world -> items[ITEM_exp] = setup_exp_item();

		// Spawn one Target
		Entity* en = entity_create();
		setup_target(en);
	}
	#endif
}

// :SerialIzation

bool world_save_to_disk() 
{
	return os_write_entire_file_s(STR("world"), (string){sizeof(World), (u8*)world});
}

bool world_attempt_load_from_disk() 
{
	string result = {0};
	bool succ = os_read_entire_file_s(STR("world"), & result, temp_allocator);
	if (!succ) 
	{
		log_error("Failed to load world.");
		return false;
	}

	// NOTE, for errors I used to do stuff like this assert:
	// assert(result.count == sizeof(World), "world size has changed!");
	//
	// But since shipping to users, I've noticed that it's always better to gracefully fail somehow.
	// That's why this function returns a bool. We handle that at the callsite.
	// Maybe we want to just start up a new world, throw a user friendly error, or whatever as a fallback. Not just crash the game lol.

	if (result.count != sizeof(World)) 
	{
		log_error("world size different to one on disk.");
		return false;
	}

	memcpy(world, result.data, result.count);
	return true;
}

// pad_pct just shrinks the rect by a % of itself ... 0.2 is a nice default

Draw_Quad* draw_sprite_in_rect(SpriteData spritedata, Range2f rect, Vector4 col, float pad_pct) 
{
	Vector2 sprite_size = get_sprite_size(spritedata);

	// make it smoller (padding)
	{
		Vector2 size = range2f_size(rect);
		Vector2 offset = rect.min;
		rect = range2f_shift(rect, v2_mulf(rect.min, -1));
		rect.min.x += size.x * pad_pct * 0.5;
		rect.min.y += size.y * pad_pct * 0.5;
		rect.max.x -= size.x * pad_pct * 0.5;
		rect.max.y -= size.y * pad_pct * 0.5;
		rect = range2f_shift(rect, offset);
	}

	// ratio render lock
	if (sprite_size.x > sprite_size.y) 
	{ // long boi

		// height is a ratio of width
		Vector2 range_size = range2f_size(rect);
		rect.max.y = rect.min.y + (range_size.x * (sprite_size.y/sprite_size.x));
		// center along the Y
		float new_height = rect.max.y - rect.min.y;
		rect = range2f_shift(rect, v2(0, (range_size.y - new_height) * 0.5));

	} 
	else if (sprite_size.y > sprite_size.x) 
	{ // tall boi
		
		// width is a ratio of height
		Vector2 range_size = range2f_size(rect);
		rect.max.x = rect.min.x + (range_size.y * (sprite_size.x/sprite_size.y));
		// center along the X
		float new_width = rect.max.x - rect.min.x;
		rect = range2f_shift(rect, v2((range_size.x - new_width) * 0.5, 0));
	}

	return draw_image(spritedata.image, rect.min, range2f_size(rect), col);
}

void do_ui_stuff()
{
	set_screen_space();

	push_z_layer(Layer_UI);

	Vector2 txt_scale = v2(0.1, 0.1);
	Vector4 bg_col = v4(0, 0, 0, 0.90);
	Vector4 fill_col = v4(0.5, 0.5, 0.5, 1.0);
	Vector4 accent_col_blue = hex_to_rgba(0x44c3daff);
	Vector4 accent_col_purple = hex_to_rgba(0x9d00ffff);

	// :Inventory UI
	{
		if(is_key_just_pressed(KEY_TAB))
		{
			consume_key_just_pressed(KEY_TAB);

			// Swap to Inventory UI Open State.
			world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);
		}

		world -> inventory_alpha_target = (world -> ux_state == UX_inventory ? 1.0 : 0.0);
		animate_f32_to_target(& world -> inventory_alpha, world -> inventory_alpha_target, delta_t, 200.0); //speed inventory closes / fades
		bool is_inventory_enabled = world -> inventory_alpha_target == 1;

		if(world -> inventory_alpha != 0.0)
		{
			// TODO make inventory fade in out when key pressed.

			float y_pos = 240.0;

			int item_count = 0;

			for (int i = 0; i < ITEM_MAX; i++)
			{
				ItemData* item = & world -> items[i];

				if (item -> amount > 0)
				{
					item_count += 1;
				}
			}

			const float icon_size = 16.0;

			const int icon_row_count = 8;

			const int icon_row_count_items = 3;

			float entire_thing_width = icon_row_count_items * icon_size;
			float x_start_pos = (screen_width * 0.5) - (entire_thing_width * 0.5);

			// Black background box
			{
				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
				draw_rect_xform(xform, v2(entire_thing_width, icon_size), bg_box_color);
			}

			int slot_index = 0;
			for (int id = 1; id < ITEM_MAX; id++)
			{
				ItemData* item = & world -> items[id];

				if (item -> amount > 0)
				{
					// Draw item icons
					float slot_index_offset = slot_index * icon_size;

					Matrix4 xform = m4_scalar(1.0);

					xform = m4_translate(xform, v3(x_start_pos + slot_index_offset, y_pos, 0.0));
					SpriteData spritedata = item -> spriteData;
					Vector2 icon_positon = v2(x_start_pos + slot_index_offset, y_pos);
					
					// White transparent box to show item slot is filled.
					Draw_Quad* quad = draw_rect_xform(xform, v2(icon_size, icon_size), v4(1, 1, 1, 0.2));

					Range2f icon_box = quad_to_range(*quad);

					float is_selected_alpha = 0.0;
				
					if (is_inventory_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc()))
					{
						is_selected_alpha = 1.0;
					}

					/*
					xform = m4_translate(xform, v3(icon_size * 0.5, icon_size * 0.5, 0.0));
	
					// Make items start slightly smaller so when sized up they dont get to big
					{
						float scale_adjust = -0.10;
						xform = m4_scale(xform, v3(1 + scale_adjust, 1 + scale_adjust, 1));
					}
					
					// Make items bigger when selected
					if (is_selected_alpha == 1.0)
					{
						// TODO selection polish
						float scale_adjust = 0.25;
						xform = m4_scale(xform, v3(1 + scale_adjust, 1 + scale_adjust, 1));
					}

					// Breathe effect on items in inventory when selected
					if (is_selected_alpha == 1.0)
					{
						float scale_adjust = 0.025 * sin_breathe(os_get_elapsed_seconds(), 5.0);
						xform = m4_scale(xform, v3(1 + scale_adjust, 1 + scale_adjust, 1));
					}

					// Rotate effect on items in inventory when selected
					if (is_selected_alpha == 1.0)
					{
						float rotate_adjust = PI32 * 0.05 * sin_breathe(os_get_elapsed_seconds(), 2.0);
						xform = m4_rotate_z(xform, rotate_adjust);
					}
					*/

					xform = m4_translate(xform, v3(icon_size * -0.5,  icon_size * -0.5, 0));

					// Draw Sprite

					Range2f box = range2f_make_bottom_left(icon_positon, v2(icon_size, icon_size));

					//draw_image_xform(sprite -> image, xform, v2(icon_size, icon_size), COLOR_WHITE); // old

					draw_sprite_in_rect(spritedata, box, COLOR_WHITE, 0.2); // New sprite rendering from randy day 11
				
					// Tooltip
					if (is_selected_alpha == 1.0)
					{
						string name = tprintf("%s", item -> pretty_name);

						string title = sprint(get_temporary_allocator(), STR("%s\nx%i"), name, item -> amount);

						//log("%s", item -> pretty_name);

						draw_tooltip_box_string_below_same_size(quad, icon_size, & title);
					}
					slot_index += 1;
				}
			}

			// Mana bar
			if (mana.unlocked == true)
			{
				float y_pos = 240;

				string name = STR("Mana");
				draw_resource_bar(y_pos, & mana.current, & mana.max, & mana.per_second, icon_size, icon_row_count, accent_col_blue, bg_box_color, & name);

				if(channel_mana.level >= 5)
				{
					wisdom.unlocked = true;

					// Unlock intellect for now for testing other things (shouldn't be unlocked yet)

					if (intellect.unlocked == false)
					{
						intellect.unlocked = true;
					}
				}
			}

			// Intellect bar
			if(intellect.unlocked == true)
			{
				float y_pos = 220;

				string name = STR("Intellect");
				draw_resource_bar(y_pos, & intellect.current, & intellect.max, & intellect.per_second, icon_size, icon_row_count, accent_col_purple, bg_box_color, & name);

				if(wisdom.level >= 5)
				{
					focus.unlocked = true;
				}
			}

			// Setup for all buttons for now
			float button_size = 16.0;

			Vector2 button_size_v2 = v2(16.0, 16.0);

			// Level Up Channel Mana Button
			if(channel_mana.unlocked == true)
			{
				Vector2 button_pos = v2(175, y_pos);

				Vector4 color = fill_col;

				string channel_button_text = sprint(get_temporary_allocator(), STR("Channel Mana\nLevel:%i\nCost: %.1f Mana"), channel_mana.level, channel_mana.current_costs[0]);

				string channel_mana_tooltip = sprint(get_temporary_allocator(), STR("Channel Mana\nLevel:%i\nCost: %.1f Mana\n+%.2f Base Mana / second\nChannel your mana to Increase\nit's recovery speed."), channel_mana.level, channel_mana.current_costs[0], channel_mana.current_effect_value);

				if(check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;

					level_up_channel_mana_if_unlocked();
				}

				Draw_Quad* quad = draw_level_up_button(channel_button_text, button_size, button_pos, color);	

				if(check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;
					color = COLOR_RED;
					draw_tooltip_box_string_to_side_larger(quad, icon_size, & channel_mana_tooltip);
				}
				//log("Level:%i, mana Regen Effect:%.2f, Power Multi: %.2f, Cost: %.2f, Cost Multiplier1: %.2f, Cost Multiplier2 %.2f", channel_mana.level, channel_mana.current_effect_value, channel_mana.current_power_multiplier, channel_mana.current_costs[0], channel_mana.cost_multipliers[0], channel_mana.cost_multipliers[1]);
			}

			// Level Up wisdom Button
			if(wisdom.unlocked == true)
			{
				Vector2 button_pos = v2(175, y_pos - 30);

				Vector4 color = fill_col;

				string wisdom_button_text = sprint(get_temporary_allocator(), STR("Wisdom\nLevel:%i\nCost: %.1f Intellect"), wisdom.level, wisdom.current_costs[0]);

				string wisdom_tooltip = sprint(get_temporary_allocator(), STR("Wisdom\nLevel:%i\nCost: %.1f Intellect\n+%.1f Max Mana\nWisdom expands your mana reserves."), wisdom.level, wisdom.current_costs[0], wisdom.current_effect_value);
				
				if(check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;

					level_up_wisdom_if_unlocked();
				}

				Draw_Quad* quad = draw_level_up_button(wisdom_button_text, button_size, button_pos, color);	

				if(check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;
					color = COLOR_RED;
					draw_tooltip_box_string_to_side_larger(quad, icon_size, & wisdom_tooltip);
				}
			}
			
			// Level Up Focus Button
			if(focus.unlocked == true)
			{
				Vector2 button_pos = v2(175, y_pos - 60);

				Vector4 color = fill_col;

				string focus_button_tooltip = sprint(get_temporary_allocator(), STR("Focus\nLevel:%i\nCost: %.1f Mana + %.1f Intellect"), focus.level, focus.current_costs[0], focus.current_costs[1]);

				string focus_tooltip = sprint(get_temporary_allocator(), STR("Focus\nLevel:%i\nCost: %.1f Mana + %.1f Intellect\n+%.1f Base Intellect / second\nPassively generate Intellect"), focus.level, focus.current_costs[0], focus.current_costs[1], focus.current_effect_value);
				
				if(check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;

					level_up_focus_if_unlocked();
				}
			
				Draw_Quad* quad = draw_level_up_button(focus_button_tooltip, button_size, button_pos, color);	

				if(check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;
					color = COLOR_RED;
					draw_tooltip_box_string_to_side_larger(quad, icon_size, & focus_tooltip);
				}
				//log("Level:%i, intellect regen Effect:%.2f, Power Multi: %.2f, Cost: %.2f, Cost Multiplier1: %.2f, Cost Multiplier2 %.2f", focus.level, focus.current_effect_value, focus.current_power_multiplier, focus.current_costs[0], focus.cost_multipliers[0], focus.cost_multipliers[1]);
			}
		}
		world_frame.hover_consumed = true;
	}

	// Esc Closes ALL UI
	if (world -> ux_state != UX_nil && is_key_just_pressed(KEY_ESCAPE)) 
	{
		consume_key_just_pressed(KEY_ESCAPE);
		world -> ux_state = 0;
	}

	set_world_space();
	pop_z_layer();
}

int entry(int argc, char **argv) 
{
	window.title = STR("Tower of the Sky");

	window.pixel_width = 1920; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.pixel_height = 1080; 
	window.fullscreen = true;

	// Where on the monitor the window starts up at
	window.x = 0;
	window.y = 0;

	window.clear_color = COLOR_BLACK;

	world = alloc(get_heap_allocator(), sizeof(World));

	float64 last_time = os_get_elapsed_seconds();

	// :Color

	Vector4 color_0 = hex_to_rgba(0x2a2d3aff);

	LoadSpriteData();

	setup_fireball_anim(); // Setup fireball animation so it can be used.

	// :Font Setup

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());

	assert(font, "Failed loading arial.ttf, %d", GetLastError());

	// Camera Settings
	float zoom = 3;
	Vector2 camera_pos = v2(0, 0);

	// world load / setup
	if (os_is_file_s(STR("world"))) 
	{
		bool succ = world_attempt_load_from_disk();
		if (!succ) 
		{
			// just setup a new world if it fails
			world_setup();
		}
	} 
	else 
	{
		world_setup();
	}

	world_save_to_disk();

	// :Game Loop

	while (!window.should_close) 
	{
		reset_temporary_storage();
		world_frame = (WorldFrame){0};

		// :Time tracking
		
		float64 current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;

		// Log fps
		if ((int)current_time != (int)last_time) log("%.2f FPS\n%.2fms", 1.0 / (current_time - last_time), (current_time - last_time) * 1000);
		last_time = current_time;

		//log("Window Width:%i, Window Height:%i", window.scaled_width, window.scaled_height);

		// find player
		for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
		{
			Entity* en = & world -> floors[world -> current_floor].entities[i];
			if (en -> is_valid && en -> entityID == ENTITY_player) 
			{
				world_frame.player = en;
				//log("player found");
			}
		}

		// :Frame Update

		draw_frame.enable_z_sorting = true;

		world_frame.world_proj = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		
		// Camera
		{
			Vector2 target_pos = get_player() -> pos;
			animate_v2_to_target(& camera_pos, target_pos, delta_t, 15.0f);

			world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0));
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0.0)));
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0)));
		}

		set_world_space();

		push_z_layer(Layer_WORLD);

		Vector2 mouse_pos_world = get_mouse_pos_in_world_space();
		int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
		int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);

		// :Do UI Rendering

		do_ui_stuff();

		// :World update
		{
			world -> time_elapsed += delta_t;
		}

		FloorData* floor_data = & world -> floors[world -> current_floor];

		render_floor_tiles(floor_data, tile_width, color_0);
		
		render_buildings(floor_data, tile_width, color_0);

		// Debug Visuals
		//update_debug_circle(& circle_state);

		// :Render Entities

		for (int i = 0; i < MAX_ENTITY_COUNT; i++)
		{
			Entity* en = & world -> floors[world -> current_floor].entities[i];

			if (en -> is_valid)
			{
				switch (en -> entityID)
				{	
					case ENTITY_player:
					{
						break;
					}

					default:
					{
						// Get sprite dimensions
						Vector2 sprite_size = get_sprite_size(sprites[en -> spriteID]);

						Matrix4 xform = m4_scalar(1.0);

						/*
						if(en -> is_item == true)
						{
							xform = m4_translate(xform, v3(0, 2 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0));
						}
						*/

						xform = m4_translate(xform, v3(en -> pos.x, en -> pos.y, 0));

						Vector4 col = COLOR_WHITE;

						draw_image_xform(sprites[en -> spriteID].image, xform, sprite_size, col);

						Vector2 health_bar_pos = v2((en -> pos.x + (sprites[en -> spriteID].image -> width * 0.5)), (en -> pos.y + (sprites[en -> spriteID].image -> height)));

						// Temp healthbar for non-players
						draw_unit_bar(health_bar_pos, & en -> health, & en -> max_health, & en -> health_regen, 4, 6, COLOR_RED, bg_box_color);

						//world space current location debug for object pos
						//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), en -> pos.x, en -> pos.y), font_height, en -> pos, v2(0.1, 0.1), COLOR_WHITE);

						break;
					}
				}
			}
		}
		//log("%f, %f,", get_player() -> pos.x, get_player() -> pos.y);

		int count = 0;

		// Remove inactive projectiles
		/*
		for (int i = MAX_PROJECTILES - 1; i >= 0; i--) 
		{
			if (!projectiles[i].is_active) 
			{
				projectiles[i] = false;
			}
		}
		*/

		// Loop through all Projectiles and render / move them.
		for (int i = 0; i < MAX_PROJECTILES; i++) 
		{		
			if (world -> floors -> projectiles[i].is_active) 
			{
				update_projectile(& world -> floors -> projectiles[i], delta_t);

				count++;
			}
		}

		if(is_key_just_pressed(KEY_F3))
		{
			//log("Active Projectiles: %i", count);
		}

		//Render player
		{
			Entity *player = get_player();
			SpriteData spritedata = sprites[player -> spriteID];

			Vector2 sprite_size = get_sprite_size(spritedata);

			Matrix4 xform = m4_scalar(1.0);

			xform = m4_translate(xform, v3(player -> pos.x, player -> pos.y, 0));

			Vector4 col = COLOR_WHITE;
			draw_image_xform(spritedata.image, xform, sprite_size, col);

			// Healthbar test values
			Vector2 health_bar_pos = v2((player -> pos.x + (spritedata.image -> width * 0.5)), (player -> pos.y + (spritedata.image -> height)));

			// Temperary render player healthbar test
			draw_unit_bar(health_bar_pos, & player -> health, & player -> max_health, & player -> health_regen, 4, 6, COLOR_RED, bg_box_color);

			// World space current location debug for object pos
			//draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), player -> pos.x, player -> pos.y), font_height, player -> pos, v2(0.2, 0.2), COLOR_WHITE);

			// floor player currently resides in
			draw_text(font, sprint(get_temporary_allocator(), STR("Current Floor:%i"), player -> current_floor), font_height, player -> pos, v2(0.2, 0.2), COLOR_WHITE);

			// Draw (REAL) Player pos as blue pixel
			//draw_rect(v2(player -> pos.x, player -> pos.y), v2(1, 1), v4(0, 255, 255, 1));

			// Draw player pos offset to be centered on the center of the sprite (NOT REAL POS)
			//draw_rect(v2((player -> pos.x + sprites[player -> spriteID].image -> width * 0.5), (player -> pos.y + sprites[player -> spriteID].image -> height * 0.5)), v2(1, 1), v4(0, 255, 0, 1));

			if(is_key_just_pressed(KEY_F3))
			{
				// Draw the debug circle around the player
            	// start_debug_circle(& circle_state, player -> pos, 80, 0.5);

				// Ghetto Fireball Spawn Test
				float fireball_cost = (mana.current - 5);

				if(mana.current >= fireball_cost)
				{
					spawn_projectile(player, 250.0, 10.0, & Fireball, 1.0, 30.0, 1000, 5);
					mana.current -= fireball_cost;
				}
			}
		}

		// Press F1 to Close Game
		if(is_key_just_pressed(KEY_F1))
		{
			window.should_close = true;
		}

		// :Wasd Movement

		Vector2 input_axis = v2(0,0);

		if (is_key_down('A')) 
		{
			input_axis.x -= 1.0;
		}

		if (is_key_down('D')) 
		{
			input_axis.x += 1.0;
		}

		if (is_key_down('S')) 
		{
			input_axis.y -= 1.0;
		}

		if (is_key_down('W')) 
		{
			input_axis.y += 1.0;
		}

		input_axis = v2_normalize(input_axis);
		
		Entity *player = get_player();

		// Update player position based on input
		updateEntity(player, v2_mulf(input_axis, 100.0 * delta_t));

		// Get player position with sprite size offset for X only
		Vector2 player_pos = v2((player -> pos.x + sprites[player -> spriteID].image -> width * 0.5), (player -> pos.y));

		// Lock player to a circle about the size of the play area.
		float player_radius = (tile_radius * tile_width) - tile_width;

		float dx = player_pos.x;
		float dy = player_pos.y;
		float distance_squared = dx * dx + dy * dy;

		// Check if the player is outside the circle
		if (distance_squared > player_radius * player_radius) 
		{
			// Normalize the player's position
			float distance = sqrtf(distance_squared);
			float scale_factor = player_radius / distance;

			// Adjust the player's position to the boundary of the circle
			player -> pos.x *= scale_factor;
			player -> pos.y *= scale_factor;
		}

		// load/save commands
		// these are at the bottom, because we'll want to have a clean spot to do this to avoid any mid-way operation bugs.
		{
			if (is_key_just_pressed('F')) 
			{
				world_save_to_disk();
				log("saved");
			}
			if (is_key_just_pressed('R')) 
			{
				world_attempt_load_from_disk();
				log("loaded ");
			}
			if (is_key_just_pressed('K') && is_key_down(KEY_SHIFT)) 
			{
				memset(world, 0, sizeof(World));
				world_setup();
				log("reset");
			}
		}

		os_update(); 
		gfx_update();
	}

	world_save_to_disk();

	return 0;
}