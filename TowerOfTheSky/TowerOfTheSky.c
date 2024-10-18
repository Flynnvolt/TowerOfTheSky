#pragma once
#include "World.c"
#include "Range.c"
#include "TextPivot.c"
#include "TutorialMath.c"
#include "AnimationData.c"
#include "Projectile.c"
#include "Abilities.c"
#include "Limits.c"
#include "Entities.c"
#include "Sprites.c"
#include "UXState.c"
#include "Item.c"
#include "Floor.c"
#include "Enemy.c"
#include "Building.c"
#include "WorldFrame.c"
#include "Player.c"
#include "Upgrades.c"

#define DEV_TESTING

#define m4_identity m4_make_scale(v3(1, 1, 1))

// :Global APP

Gfx_Font* font;

u32 font_height = 48;

float screen_width = 480.0;

float screen_height = 270.0;

Draw_Frame* current_draw_frame = 0;

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

Vector4 bg_box_color = {0, 0, 0, 1};

const float icon_size = 16;

const Vector2 button_size = {64, 16};

const Vector2 tooltip_size = {80, 32};

float camera_zoom = 3;

Vector2 camera_pos = {0};

Vector4 color_0;

// :Timing

float64 delta_t;

float64 current_time;

float64 last_time;

// :Fps display
#define FPS_BUFFER_SIZE 10 

const float64 display_update_interval = 0.1; 
float64 display_timer = 0.0;      
int fps_buffer[FPS_BUFFER_SIZE] = {0}; 
int fps_index = 0; 
int frame_count = 0; 
float total_frame_time = 0.0f; 

int fps_display = 0;
float frame_time_display = 0.0f;

typedef enum View_Mode View_Mode;

enum View_Mode 
{
	VIEW_GAME_AFTER_POSTPROCESS,
	VIEW_GAME_BEFORE_POSTPROCESS,
	VIEW_BLOOM_MAP,
	VIEW_MODE_MAX
};

typedef struct Scene_Cbuffer Scene_Cbuffer;

struct Scene_Cbuffer 
{
	Vector2 mouse_pos_screen; // We use this to make a light around the mouse cursor
	Vector2 window_size; // We only use this to revert the Y in the shader because for some reason d3d11 inverts it.
};

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

void update_cooldown(float *cooldown) 
{
    if (*cooldown > 0) 
    {
        *cooldown -= delta_t;
        if (*cooldown < 0) 
        {
            *cooldown = 0;
        }
    }
}

Vector2 get_mouse_pos_in_current_space() 
{
	float mouse_x = input_frame.mouse_x;
	float mouse_y = input_frame.mouse_y;

	// this is a bit icky, but we're using the draw frame's matricies if valid. If not, we default to world space
	Matrix4 proj;
	if (current_draw_frame) 
	{
		proj = current_draw_frame->projection;
	} else {
		proj = world_frame.world_proj;
	}
	Matrix4 view;
	if (current_draw_frame) 
	{
		view = current_draw_frame->camera_xform;
	} else {
		view = world_frame.world_view;
	}

	float window_w = window.width;
	float window_h = window.height;

	// Normalize the mouse coordinates
	float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
	float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

	// Transform to world coordinates
	Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
	world_pos = m4_transform(m4_inverse(proj), world_pos);
	world_pos = m4_transform(view, world_pos);
	// log("%f, %f", world_pos.x, world_pos.y);

	// Return as 2D vector
	return (Vector2){ world_pos.x, world_pos.y };
}

int get_fps()
{
    if (delta_t > 0.0) 
	{
        return (int)(1.0 / delta_t); 
    }
    return 0;
}

void calculate_fps()
{
	int current_fps = get_fps(); 

	fps_buffer[fps_index] = current_fps; 

	fps_index = (fps_index + 1) % FPS_BUFFER_SIZE;

	int total_fps = 0;

	for (int i = 0; i < FPS_BUFFER_SIZE; i++) 
	{
		total_fps += fps_buffer[i];
	}

	int average_fps = total_fps / FPS_BUFFER_SIZE;

	float frame_time = current_time - last_time;
	total_frame_time += frame_time; 
	frame_count++; 

	display_timer += delta_t; 

	if (display_timer >= display_update_interval) 
	{
		frame_time_display = (total_frame_time / frame_count) * 1000; // Average in ms
		fps_display = average_fps;
		total_frame_time = 0.0f; 
		frame_count = 0; 
		display_timer = 0.0; 
	}
}

// :Debugging Tools

void collide_visual_debug(Entity *current_entity, Draw_Frame *frame)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity *actor = & world -> floors[world -> current_floor].enemies[i].enemy_entity;

		if (actor -> is_valid && actor != current_entity) 
		{
			SpriteData sprite_data = sprites[actor -> sprite_ID];
			int sprite_width = sprite_data.image -> width;
			int sprite_height = sprite_data.image -> height;

			SpriteData sprite_data_2 = sprites[current_entity -> sprite_ID];

			// Visual Debug tools
			draw_rect_in_frame(v2(actor -> pos.x, actor -> pos.y), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2), frame);  // Draw bounding box
			draw_rect_in_frame(v2(current_entity -> pos.x, current_entity -> pos.y), v2(sprite_data_2.image -> width, sprite_data_2.image -> height), v4(255, 0, 0, 0.2), frame);  // Draw bounding box
			draw_rect_in_frame(v2(current_entity -> pos.x, current_entity -> pos.y), v2(1, 1), v4(0, 255, 255, 1), frame); // Where we are
		}
	}
}

void collide_visual_debug_buildings(Entity *current_entity, Draw_Frame *frame)
{
	float half_tile_width = tile_width * 0.5f;

    for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
        BuildingData *building = & world -> floors[world -> current_floor].tiles[i].building;

        if (building -> is_valid) 
        {
            SpriteData sprite_data = building -> sprite_data;
            int sprite_width = sprite_data.image -> width;
            int sprite_height = sprite_data.image -> height;

            // Visual Debug tools
            draw_rect_in_frame(v2(building -> pos.x - half_tile_width, building -> pos.y - half_tile_width), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2), frame);  // Draw bounding box
        }
    }
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

void update_debug_circle(DebugCircleState *state, Draw_Frame *frame) 
{
    if (state -> active) 
    {
        if (state -> time_remaining > 0.0f) 
        {
            Vector2 circle_size = v2(state -> radius * 2.0f, state -> radius * 2.0f);
            Vector4 circle_color = v4(255, 0, 0, 1);

			// Center it on the position
            draw_circle_in_frame(v2(state -> center.x - state -> radius, state -> center.y - state -> radius), circle_size, circle_color, frame);

            // Draw the current position of the circle for debugging
            draw_text_in_frame(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), state -> center.x, state -> center.y), font_height, state -> center, v2(0.2, 0.2), COLOR_WHITE, frame);

            state -> time_remaining -= delta_t;

            if (state -> time_remaining <= 0.0f) 
            {
                state -> active = false;
            }
        }
    }
}

// :Entity

Entity* entity_create() 
{
	Entity* entity_found = 0;

	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity* existing_entity = & world -> floors[world -> current_floor].enemies[i].enemy_entity;

		if (!existing_entity -> is_valid) 
		{
			entity_found = existing_entity;
			break;
		}
	}
	//log("%i", world -> current_floor);
	assert(entity_found, "No more free entities!");

	entity_found -> is_valid = true;
	entity_found -> current_floor = world -> current_floor;

	return entity_found;
}

Enemy* enemy_create()
{
	Enemy* enemy_found = 0;

	// Search for an available enemy slot
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Enemy* existing_enemy = & world -> floors[world -> current_floor].enemies[i];

		if (!existing_enemy -> enemy_entity.is_valid) 
		{
			enemy_found = existing_enemy;
			break;
		}
	}

	// log("%i", world -> current_floor);
	assert(enemy_found, "No more free enemies!");

	enemy_found -> enemy_entity.is_valid = true;
	enemy_found -> enemy_entity.current_floor = world -> current_floor;

	return enemy_found;
}

void entity_destroy(Entity* entity) 
{
	memset(entity, 0, sizeof(Entity));
}

// :Floor System

void create_circle_floor_data(FloorData* floor)
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
				tile_data -> is_valid = true;

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

// :Setup

void setup_slime(Enemy* en)
{
    *en = slime_defaults;
    en -> enemy_entity.current_floor = world -> current_floor;
}

void setup_target(Entity* en) 
{
	en -> entity_ID = ENTITY_Enemy;
    en -> sprite_ID = SPRITE_Target;
	en -> health = 100;
	en -> max_health = 100;
	en -> health_regen = 25;
	en -> is_immortal = true;
	en -> pos = v2(0, 100);
	en -> pos = round_v2_to_tile(en -> pos);
	en -> pos.y -= tile_width * 0.5;
	en -> pos.x -= sprites[en -> sprite_ID].image -> width * 0.5;
}

ItemData setup_exp_item() 
{
    ItemData item;
    item.item_ID = ITEM_Exp;
    item.sprite_data = sprites[SPRITE_Exp];
    item.amount = 50;
    item.is_valid = true;
    strcpy(item.pretty_name, "EXP"); 
    strcpy(item.description, "Experience points for leveling up"); 
    return item;
}

BuildingData setup_building_stairs_up()
{
    BuildingData building;
    building.building_ID = BUILDING_Stairs_Up;
    building.sprite_data = sprites[SPRITE_Stairs_Up];
    building.is_valid = true;
    strcpy(building.pretty_name, "Stairs that go up"); 
    strcpy(building.description, "Travel between floors"); 
    return  building;
}

BuildingData setup_building_stairs_down() 
{
    BuildingData building;
    building.building_ID = BUILDING_stairs_Down;
    building.sprite_data = sprites[SPRITE_Stairs_Down];
    building.is_valid = true;
    strcpy(building.pretty_name, "Stairs that go down"); 
    strcpy(building.description, "Travel between floors"); 
    return  building;
}

BuildingData setup_building_crate() 
{
    BuildingData building;
    building.building_ID = BUILDING_Crate;
    building.sprite_data = sprites[SPRITE_Crate];
    building.is_valid = true;
    strcpy(building.pretty_name, "crate"); 
    strcpy(building.description, "maybe loot?"); 
    return  building;
}

BuildingData setup_building_wall() 
{
    BuildingData building;
    building.building_ID = BUILDING_Wall;
    building.sprite_data = sprites[SPRITE_Wall];
    building.is_valid = true;
    strcpy(building.pretty_name, "wall"); 
    strcpy(building.description, "a sturdy wall"); 
    return  building;
}

void setup_stairs(FloorData *floor, bool first_floor, int floor_ID)
{
    for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
        TileData tile_data = floor->tiles[i];
        
        int x = tile_data.tile.x; 
        int y = tile_data.tile.y;

        // Place a staircase up at 5, 5
        if (x == 5 && y == 5)
        {
			if (is_even(floor_ID))
			{
				floor -> tiles[i].building = setup_building_stairs_up();
			}
			else
			{
				floor -> tiles[i].building = setup_building_stairs_down();
			}
			
            floor -> tiles[i].building.pos = v2((x * tile_width), (y * tile_width));
			floor -> tiles[i].building.current_floor = floor_ID;
        }

        if (!first_floor)
        {
            // Place a staircase down at -5, -5
            if (x == -5 && y == -5)
			{
				if (is_even(floor_ID))
				{
					floor -> tiles[i].building = setup_building_stairs_down();
				}
				else
				{
					floor -> tiles[i].building = setup_building_stairs_up();
				}

				floor -> tiles[i].building.pos = v2((x * tile_width), (y * tile_width));
				floor -> tiles[i].building.current_floor = floor_ID;
            }
        }
    }
}

void setup_crates(FloorData *floor, int num_crates, int floor_ID)
{
    int placed_crates = 0;

    while (placed_crates < num_crates)
    {
        int i = rand() % MAX_TILE_COUNT;
        
        TileData *tile_data = & floor -> tiles[i];

        int x = tile_data -> tile.x;
        int y = tile_data -> tile.y;

        // Check if the tile already has a building
        if (tile_data -> building.is_valid == true)
        {
            continue;
        }

        // don't place crates in the center of floor
        if (x >= -6 && x <= 6 && y >= -6 && y <= 6)
        {
            continue;
        }

		float floor_radius = 30;  
		float edge_thickness = 5;  

		float distance_from_center = sqrtf(x * x + y * y);

		// don't place crates on the edges
		if (distance_from_center >= (floor_radius - edge_thickness) && 
			distance_from_center <= floor_radius)
		{
			continue;
		}

        tile_data -> building = setup_building_crate();
        tile_data -> building.pos = v2((x * tile_width), (y * tile_width));
        tile_data -> building.current_floor = floor_ID;

        placed_crates++;
    }
}

void setup_walls(FloorData *floor, int floor_ID)
{
    for (int i = 0; i < MAX_TILE_COUNT; i++)
    {
        TileData *tile_data = & floor -> tiles[i];

        int x = tile_data -> tile.x;
        int y = tile_data -> tile.y;

        float dx = (float)x;
        float dy = (float)y;
        float distance = sqrtf(dx * dx + dy * dy); 

        if (distance >= (tile_radius - 1.0f) && distance <= tile_radius && tile_data -> building.is_valid == false)
        {
            tile_data -> building = setup_building_wall();
            tile_data -> building.pos = v2((x * tile_width), (y * tile_width));
            tile_data -> building.current_floor = floor_ID;
        }
    }
}

// :Player

Entity* get_player() 
{
	if (world -> player.player.is_valid == true)
	{
		return & world -> player.player;
	}

    return NULL;
}

void setup_player(Player* player_en)
{
    *player_en = player_hero;

    player_en -> player.pos = round_v2_to_tile(player_en -> player.pos);
    player_en -> player.pos.y -= tile_width * 0.5;
    player_en -> player.pos.x -= sprites[player_en -> player.sprite_ID].image -> width * 0.5;
	player_en -> player.current_floor = world -> current_floor;

	// Center X and Y
	//player_en -> pos = v2((player_en -> pos.x - sprites[player_en -> spriteID].image -> width * 0.5),(player_en -> pos.y - sprites[player_en -> spriteID].image -> height * 0.5));
}

Resource* get_player_resource(ResourceID resource_ID)
{
	for (int i = 0; i < RESOURCEID_MAX; i++)
	{
		if (world -> player.resource_list[i].resource_ID == resource_ID)
		{
			return & world -> player.resource_list[i];
		}
	}
	return NULL;
}

bool is_resource_in_player_list(ResourceID resource_ID)
{
    for (int i = 0; i < RESOURCEID_MAX; i++)
    {
        if (world -> player.resource_list[i].resource_ID == resource_ID)
        {
            return true; 
        }
    }
    return false; 
}

void add_player_resource(ResourceID resource_ID)
{
    if (is_resource_in_player_list(resource_ID) == resource_ID)
    {
        log("Resource '%s' is already in the player's resource list.\n", resources[resource_ID].name);
        return;
    }

    for (int i = 0; i < RESOURCEID_MAX; i++)
    {
		if (world -> player.resource_list[i].resource_ID == RESOURCEID_nil)
        {
            world -> player.resource_list[i] = resources[resource_ID];
            world -> player.resource_list[i].unlocked = true;

            log("Resource '%s' added to player's resource list.\n", resources[resource_ID].name);

            return;
        }
    }

    log("Player's skill list is full, cannot add skill '%s'.\n", resources[resource_ID].name);
}


Skill* get_player_skill(SkillID skill_ID)
{
	for (int i = 0; i < SKILLID_MAX; i++)
	{
		if (world -> player.skill_list[i].skill_ID == skill_ID)
		{
			return & world -> player.skill_list[i];
		}
	}
	return NULL;
}

bool is_skill_in_player_list(SkillID skill_ID)
{
    for (int i = 0; i < SKILLID_MAX; i++)
    {
        if (world -> player.skill_list[i].skill_ID == skill_ID)
        {
            return true; 
        }
    }
    return false; 
}

void add_player_skill(SkillID skill_ID)
{
    if (is_skill_in_player_list(skill_ID))
    {
        log("Skill '%s' is already in the player's skill list.\n", skills[skill_ID].name);
        return;
    }

    for (int i = 0; i < SKILLID_MAX; i++)
    {
        if (world -> player.skill_list[i].skill_ID == SKILLID_nil)
        {
            world -> player.skill_list[i] = skills[skill_ID];
            world -> player.skill_list[i].unlocked = true;

            log("Skill '%s' added to player's skill list.\n", skills[skill_ID].name);

            return;
        }
    }

    log("Player's skill list is full, cannot add skill '%s'.\n", skills[skill_ID].name);
}

void find_skill_to_level(SkillID skill_ID)
{
    Skill* skill = get_player_skill(skill_ID);  

    if (skill != NULL && skill -> unlocked)  
    {
        skill -> level_up(skill, world -> player.resource_list);
    }
}
Ability* get_player_ability(AbilityID ability_ID)
{
	for (int i = 0; i < ABILITYID_MAX; i++)
	{
		if (world -> player.ability_list[i].ability_ID == ability_ID)
		{
			return & world -> player.ability_list[i];
		}
	}
	return NULL;
}

void find_ability_to_level(AbilityID ability_ID)
{
    Ability* ability = get_player_ability(ability_ID);  

    if (ability != NULL && ability -> unlocked)  
    {
        ability -> level_up(ability);
    }
}

bool is_ability_in_player_list(AbilityID ability_ID)
{
    for (int i = 0; i < ABILITYID_MAX; i++)
    {
        if (world -> player.ability_list[i].ability_ID == ability_ID)
        {
            return true; 
        }
    }
    return false; 
}

void add_player_ability(AbilityID ability_ID)
{
    if (is_ability_in_player_list(ability_ID))
    {
        log("Ability '%s' is already in the player's ability list.\n", abilities[ability_ID].name);
        return;
    }

    for (int i = 0; i < ABILITYID_MAX; i++)
    {
        if (world -> player.ability_list[i].ability_ID == ABILITYID_Nil)
        {
            world -> player.ability_list[i] = abilities[ability_ID];
            world -> player.ability_list[i].unlocked = true;

            log("Ability '%s' added to player's ability list.\n", abilities[ability_ID].name);

            return;
        }
    }

    log("Player's Ability list is full, cannot add ability '%s'.\n", abilities[ability_ID].name);
}

bool is_upgrade_in_known(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.known_upgrades[i].upgrade_ID == upgrade_ID)
        {
            return true; // Upgrade already exists
        }
    }
    return false; // Upgrade not found
}

void update_known_upgrades()
{
    int known_count = 0;

    // Clear unlocked upgrades
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.known_upgrades[i].unlocked == true)
        {
            world -> player.known_upgrades[i].known = false;
        }
    }

    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.all_upgrades[i].known && !world -> player.all_upgrades[i].unlocked)
        {
            // Check if upgrade is already in known
            if (!is_upgrade_in_known(world -> player.all_upgrades[i].upgrade_ID))
            {
                world -> player.known_upgrades[known_count] = world -> player.all_upgrades[i];
                known_count++;
            }
        }
    }
}

void mark_upgrade_unlocked(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.all_upgrades[i].upgrade_ID == upgrade_ID)
        {
            world -> player.all_upgrades[i].unlocked = true;
            //log("Upgrade '%s' unlocked!\n", upgrades[i].name);
            return;
        }
    }
    log("Upgrade with ID %i not found.\n", upgrade_ID);
}

void mark_upgrade_known(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.all_upgrades[i].upgrade_ID == upgrade_ID)
        {
            world -> player.all_upgrades[i].known = true;
            //log("Upgrade '%s' known!\n", upgrades[i].name);
            return;
        }
    }
    log("Upgrade with ID %i not found.\n", upgrade_ID);
}

Upgrade* get_player_upgrade(UpgradeID upgrade_ID)
{
	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (world -> player.upgrade_list[i].upgrade_ID == upgrade_ID)
		{
			return & world -> player.upgrade_list[i];
		}
	}
	return NULL;
}

bool is_upgrade_in_player_list(UpgradeID upgrade_ID)
{
    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.upgrade_list[i].upgrade_ID == upgrade_ID)
        {
            return true; // Upgrade already exists in player's list
        }
    }
    return false; // Upgrade not found in player's list
}

bool is_upgrade_in_ability_upgrade_list(AbilityUpgrade ability_upgrades[UPGRADEID_MAX], UpgradeID upgrade_ID)
{
	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (ability_upgrades[i].ability_upgrade_ID == upgrades[upgrade_ID].ability_upgrade.ability_upgrade_ID)
		{
			return true;
		}
	}
	return false;
}

AbilityUpgrade* get_upgrade_in_ability_upgrade_list(AbilityUpgrade ability_upgrades[UPGRADEID_MAX], UpgradeID upgrade_ID)
{
	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (ability_upgrades[i].ability_upgrade_ID == upgrades[upgrade_ID].ability_upgrade.ability_upgrade_ID)
		{
			return & ability_upgrades[i];
		}
	}
	return NULL;
}

bool is_ability_id_in_list(AbilityID* ability_list, AbilityID ability_ID)
{
    for (int i = 0; i < ABILITYID_MAX; i++) 
    {
		//log("%i", ability_list[i]);
		//log("Ability id: %i", ability_ID);
        if (ability_list[i] == ability_ID) 
        {
            return true;
        }
    }
    return false;
}

void upgrade_abilities(AbilityID abilities_list[ABILITYID_MAX], UpgradeID upgrade_ID)
{
	for (int i = 0; i < ABILITYID_MAX; i++)
	{
		if (abilities_list[i] == ABILITYID_Nil)
		{
			continue;
		}
		
		if (is_ability_in_player_list(abilities_list[i]) == true)
		{
			//log("%i", get_player_upgrade(upgrade_ID) -> abilities_unlocked[0]);

			//log("Abilties list: %i", abilities_list[i]);

			if (is_ability_id_in_list(get_player_upgrade(upgrade_ID) -> abilities_unlocked, abilities_list[i]) == true)
			{
				find_ability_to_level(abilities_list[i]);
				//log("%s", upgrades[upgrade_ID].name);
				break;
			}
			
			if (is_upgrade_in_ability_upgrade_list(get_player_ability(abilities_list[i]) -> ability_upgrades, upgrade_ID) == true)
			{
				for (int j = 0; j < UPGRADEID_MAX; j++)
				{
					if (get_player_ability(abilities_list[i]) -> ability_upgrades[j].active == true)
					{
						get_player_ability(abilities_list[i]) -> ability_upgrades[j].level++;
					}
				}
			}
			else
			{
				for (int j = 0; j < UPGRADEID_MAX; j++)
				{
					if (get_player_ability(abilities_list[i]) -> ability_upgrades[j].active == false)
					{
						get_player_ability(abilities_list[i]) -> ability_upgrades[j] = upgrades[upgrade_ID].ability_upgrade;
						get_player_ability(abilities_list[i]) -> ability_upgrades[j].active = true;
						get_player_ability(abilities_list[i]) -> ability_upgrades[j].level++;
					}
				}
			}
		}
	}
}

void level_up_upgrade(UpgradeID upgrade_ID)
{
	if (is_upgrade_in_player_list(upgrade_ID) == true)
	{
		if (get_player_upgrade(upgrade_ID) -> has_levels == true)
		{
			get_player_upgrade(upgrade_ID) -> level++;
			log("Upgrade: %s leveled up", upgrades[upgrade_ID].name);
			upgrade_abilities(get_player_upgrade(upgrade_ID) -> abilities_upgraded, upgrade_ID);
		}
	}
}

void add_upgrade_to_player(UpgradeID upgrade_ID)
{
    if (is_upgrade_in_player_list(upgrade_ID))
    {
        log("Upgrade '%i' is already in the player's upgrade list.\n", upgrade_ID);
        return;
    }

    for (int i = 0; i < UPGRADEID_MAX; i++)
    {
        if (world -> player.upgrade_list[i].upgrade_ID == UPGRADEID_nil)
        {
            world -> player.upgrade_list[i] = upgrades[upgrade_ID];
			world -> player.upgrade_list[i].unlocked = true;

			level_up_upgrade(upgrade_ID);

            log("Upgrade '%s' added to player's upgrade list.\n", upgrades[upgrade_ID].name);

            return;
        }
    }

    log("Player's upgrade list is full, cannot add upgrade '%s'.\n", upgrades[upgrade_ID].name);
}

void player_change_floor(int target_floor) 
{
	world -> player.player.current_floor = target_floor;

    log("Player transferred to floor %i", target_floor);
}

// :Enemy

void spawn_enemies(SpriteID enemy_ID, FloorData *floor, float spawn_chance)
{
    for (int i = 0; i < MAX_TILE_COUNT; i++) 
    {
		if (floor -> tiles[i].is_valid == true)
		{
			TileData *tile_data = & floor -> tiles[i];

			int x = tile_data -> tile.x; 
			int y = tile_data -> tile.y;

			// skip if building is on tile
			if (tile_data -> building.is_valid == true)
			{
				continue;
			}

			float floor_radius = 30;  
			float edge_thickness = 5;  

			float distance_from_center = sqrtf(x * x + y * y);

			// don't place enemies on the edges
			if (distance_from_center >= (floor_radius - edge_thickness) && 
				distance_from_center <= floor_radius)
			{
				continue;
			}

			// 0 or 1
			float random_value = (float)rand() / RAND_MAX;

			if (random_value < spawn_chance) 
			{
				Enemy* enemy = enemy_create();

				switch (enemy_ID) 
				{
					case SPRITE_Slime:
					{
						setup_slime(enemy);
						break;
					}

					default: 
					{
						log_error("no enemy of type"); 
						break;
					}
				}

				enemy -> enemy_entity.pos = v2((x * tile_width), (y * tile_width));
			}
		}
		else
		{
			continue;
		}
    }
}

// Floor System

FloorData* create_empty_floor(bool first_floor, int floor_ID)
{
    FloorData* floor = (FloorData*) alloc(get_temporary_allocator(), sizeof(FloorData));
    memset(floor, 0, sizeof(FloorData));

    create_circle_floor_data(floor);
    setup_walls(floor, floor_ID);
    setup_stairs(floor, first_floor, floor_ID);

    if (first_floor == false)
    {
        setup_crates(floor, 10, floor_ID);
    }

    floor -> is_valid = true;
    floor -> floor_ID = floor_ID;

    return floor;
}

void load_next_floor()
{
	int next_floor_id = world -> current_floor + 1;

	if (next_floor_id < MAX_FLOOR_COUNT)
	{
		if (world -> floors[next_floor_id].is_valid == true)
		{
			player_change_floor(next_floor_id);
			memset(world -> projectiles, 0, sizeof(world -> projectiles));
			world -> current_floor++;
		}
		else
		{
			world -> floors[next_floor_id] = *create_empty_floor(false, next_floor_id);
			world -> active_floors++;
			player_change_floor(next_floor_id);
			memset(world -> projectiles, 0, sizeof(world -> projectiles));
			world -> current_floor++;
			spawn_enemies(SPRITE_Slime, & world -> floors[world -> current_floor], 0.005);
		}
	}
	else
	{
		log("tried to exceed max floors");
	}
}

void load_previous_floor()
{
	int next_floor_id = world -> current_floor - 1;

	if (next_floor_id >= 0)
	{
		if (world -> floors[next_floor_id].is_valid == true)
		{
			player_change_floor(next_floor_id);
			memset(world -> projectiles, 0, sizeof(world -> projectiles));
			world -> current_floor--;
		}
		else
		{
			world -> floors[next_floor_id] = *create_empty_floor(false, next_floor_id);
			world -> active_floors++;
			player_change_floor(next_floor_id);
			memset(world -> projectiles, 0, sizeof(world -> projectiles));
			world -> current_floor--;
			spawn_enemies(SPRITE_Slime, & world -> floors[world -> current_floor], 0.005);
		}
	}
	else
	{
		log("tried to go to a negitive floor");
	}
}

// Collision

bool collide_at(Entity *current_entity, int x, int y) 
{
	SpriteData sprite_data = sprites[current_entity -> sprite_ID];
	int width1 = sprite_data.image -> width;
	int height1 = sprite_data.image -> height;

	int x_end1 = x + width1;
	int y_end1 = y + height1;

	// check player collide

	Entity *actor = & world -> player.player;

	// Skip ourselves to avoid self-collision
	if (actor -> is_valid && actor != current_entity) 
	{
		SpriteData sprite_data_2 = sprites[actor -> sprite_ID];
		int width2 = sprite_data_2.image -> width;
		int height2 = sprite_data_2.image -> height;

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

	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity *actor = & world -> floors[world -> current_floor].enemies[i].enemy_entity;

		// Skip ourselves to avoid self-collision
		if (actor -> is_valid && actor != current_entity) 
		{
			SpriteData sprite_data_2 = sprites[actor -> sprite_ID];
			int width2 = sprite_data_2.image -> width;
			int height2 = sprite_data_2.image -> height;

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
	
	// Building Collide

	float half_tile_width = tile_width * 0.5f;

	for (int i = 0; i < MAX_TILE_COUNT; i++) 
	{
		BuildingData *building = & world -> floors[world -> current_floor].tiles[i].building;

		if (building -> is_valid)
		{
			SpriteData sprite_data_3 = building -> sprite_data;
			int width3 = sprite_data_3.image -> width;
			int height3 = sprite_data_3.image -> height;

			int building_x_start = building -> pos.x - half_tile_width;
			int building_y_start = building -> pos.y - half_tile_width;
			int building_x_end = building_x_start + width3;
			int building_y_end = building_y_start + height3;
			
			// Check for bounding box overlap
			if (x < building_x_end && x_end1 > building_x_start &&
				y < building_y_end && y_end1 > building_y_start) 
			{
				//log("Collision detected with building %d\n", i);

				if (building -> building_ID == BUILDING_Stairs_Up)
				{
					if (current_entity -> entity_ID == ENTITY_Player)
					{
						if (world -> floor_cooldown <= 0)
						{
							load_next_floor();
							world -> floor_cooldown = 1.0f;
						}
					}
				}

				if (building -> building_ID == BUILDING_stairs_Down)
				{
					if (current_entity -> entity_ID == ENTITY_Player)
					{
						if (world -> floor_cooldown <= 0)
						{
							load_previous_floor();
							world -> floor_cooldown = 1.0f;
						}
					}
				}
				
				return true;
			}
		}
	}

	// No collisions detected
	return false;
}

Entity* projectile_collides_with_entity(Projectile *projectile) 
{
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity *entity = & world -> floors[world -> current_floor].enemies[i].enemy_entity;

		if (entity -> is_valid && entity != projectile -> source_entity) // Skip the caster
		{
			SpriteData sprite_data = sprites[entity -> sprite_ID];
			int entity_width = sprite_data.image -> width;
			int entity_height = sprite_data.image -> height;

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

// :Movement Calc

void move_entity_x(Entity *entity, float amount) 
{
    entity -> x_remainder += amount;
    int move = roundf(entity -> x_remainder);

    if (move != 0) 
    {
        entity -> x_remainder -= move;
        int movement_direction = (move > 0) - (move < 0);

        if (!collide_at(entity, entity -> pos.x + movement_direction, entity -> pos.y)) 
        {
            entity -> pos.x += movement_direction;
            move -= movement_direction;
        }
    }
}

void move_entity_y(Entity *entity, float amount) 
{
    entity -> y_remainder += amount;
    int move = roundf(entity -> y_remainder);

    if (move != 0) 
    {
        entity -> y_remainder -= move;
        int movement_direction = (move > 0) - (move < 0);

        if (!collide_at(entity, entity -> pos.x, entity -> pos.y + movement_direction)) 
        {
            entity -> pos.y += movement_direction;
            move -= movement_direction;
        }
    }
}

void update_entity(Entity *entity, Vector2 movement) 
{
	move_entity_x(entity, movement.x);

	move_entity_y(entity, movement.y);

	//collide_visual_debug(entity);

	//collide_visual_debug_buildings(entity);
}

// :Damage Calcs

void player_death() 
{
	world -> current_floor = 0;
	world -> player.player.pos = v2(0, 0);
	world -> player.player.pos = round_v2_to_tile(world -> player.player.pos);
	world -> player.player.pos.y -= tile_width * 0.5;
	world -> player.player.pos.x -= sprites[world -> player.player.sprite_ID].image -> width * 0.5;
}

void enemy_death(Entity *entity)
{
	entity -> is_valid = false;
}

void damage_enemy(Entity *entity, float damage)
{	
	entity -> health -= damage;

	if (entity -> health <= 0)
	{
		if (entity -> is_immortal == true)
		{
			entity -> health = entity -> max_health;
		}
		else
		{
			enemy_death(entity);
		}
	}
}

void damage_player(float damage)
{
	get_player() -> health -= damage;

	if (get_player() -> health <= 0)
	{
		player_death();
	}
}

void damage_entity(Entity *entity, float damage)
{
	EntityID entity_ID = entity -> entity_ID;

	switch (entity_ID) 
	{
		case ENTITY_Player:
		{
			damage_player(damage);
			break;
		}

		case ENTITY_Enemy:
		{
			damage_enemy(entity, damage);
			break;
		}
		
		default: 
		{
			log_error("misconfigured entity ID in Damage Entity"); 
			break;
		}
	}
}

// :Enemy AI

void update_enemy_movement(Enemy *enemy)
{
    EnemyID enemy_ID = enemy -> enemy_logic.enemy_ID;

    Entity *player = get_player();

    Vector2 player_center = v2((player -> pos.x + (sprites[player -> sprite_ID].image -> width * 0.5f)), (player -> pos.y + (sprites[player -> sprite_ID].image -> height * 0.5f)));

	Vector2 enemy_center = v2((enemy-> enemy_entity.pos.x + (sprites[enemy -> enemy_entity.sprite_ID].image -> width * 0.5f)), (enemy-> enemy_entity.pos.y + (sprites[enemy -> enemy_entity.sprite_ID].image -> height * 0.5f)));

    float distance_to_player = v2_distance(enemy_center, player_center);

    Vector2 player_direction = v2_sub(player_center, enemy -> enemy_entity.pos);
    float length = v2_length(player_direction);

    if (length != 0.0f)
    {
        player_direction = v2_scale(player_direction, 1.0f / length);
    }

	switch (enemy -> enemy_logic.enemy_state)
	{	
		case ENEMYSTATE_idle:
		{
			break;
		}

		case ENEMYSTATE_sleep:
		{
			break;
		}

		case ENEMYSTATE_patrol:
		{
			if (enemy -> enemy_logic.roam_time <= 0)
			{
				enemy -> enemy_logic.roam_time = 3;

				// Random direction for wandering
				float random_angle = (float)(rand() % 360) * (PI32 / 180.0f);
				enemy -> enemy_logic.direction.x = cosf(random_angle);
				enemy -> enemy_logic.direction.y = sinf(random_angle);
			}

			enemy -> enemy_entity.velocity = v2_scale(enemy -> enemy_logic.direction, enemy -> enemy_entity.speed);
		
			break;
		}

		case ENEMYSTATE_combat:
		{
			enemy -> enemy_logic.direction = player_direction;

			if (distance_to_player <= enemy -> enemy_logic.attack_range * 2)
			{
				// Rush in
				if (enemy -> enemy_logic.current_attack_cooldown <= 0)
				{
					enemy -> enemy_entity.speed = get_enemy_defaults(enemy_ID).enemy_entity.speed * 10;
				}
			}

			if (distance_to_player <= enemy -> enemy_logic.attack_range)
			{
				// Attack
				if (enemy -> enemy_logic.current_attack_cooldown <= 0)
				{
					enemy -> enemy_logic.current_attack_cooldown = enemy -> enemy_logic.attack_cooldown;
					damage_player(enemy -> enemy_logic.damage);
					enemy -> enemy_entity.speed = get_enemy_defaults(enemy_ID).enemy_entity.speed;
				}
			}

			enemy -> enemy_entity.velocity = v2_scale(enemy -> enemy_logic.direction, enemy -> enemy_entity.speed);

			break;
		}

		case ENEMYSTATE_flee:
		{
			Vector2 flee_direction = v2_add(player_center, enemy -> enemy_entity.pos);
			
			float length = v2_length(flee_direction);

			if (length != 0.0f)
			{
				flee_direction = v2_scale(flee_direction, 1.0f / length);
			}

			enemy -> enemy_logic.direction = flee_direction;

			enemy -> enemy_entity.velocity = v2_scale(enemy -> enemy_logic.direction, enemy -> enemy_entity.speed);

			break;
		}

		case ENEMYSTATE_knockback:
		{
			enemy -> enemy_entity.velocity = v2_mulf(player_direction, -500);

			break;
		}

		default:
		{
			break;
		}
	}

	Vector2 movement = v2_scale(enemy -> enemy_entity.velocity, delta_t);
    update_cooldown(& enemy -> enemy_logic.current_attack_cooldown);
    update_cooldown(& enemy -> enemy_logic.roam_time);
    update_entity(& enemy -> enemy_entity, movement);
}

void update_enemy_states(Enemy *enemy)
{
    if (enemy -> enemy_entity.is_valid == false) return;

    EnemyID enemy_ID = enemy -> enemy_logic.enemy_ID;

    Entity *player = get_player();

    Vector2 player_center = v2((player -> pos.x + (sprites[player -> sprite_ID].image -> width * 0.5f)), (player -> pos.y + (sprites[player -> sprite_ID].image -> height * 0.5f)));

	Vector2 enemy_center = v2((enemy-> enemy_entity.pos.x + (sprites[enemy -> enemy_entity.sprite_ID].image -> width * 0.5f)), (enemy-> enemy_entity.pos.y + (sprites[enemy -> enemy_entity.sprite_ID].image -> height * 0.5f)));

    float distance_to_player = v2_distance(enemy_center, player_center);

    Vector2 player_direction = v2_sub(player_center, enemy -> enemy_entity.pos);
    float length = v2_length(player_direction);

    if (length != 0.0f)
    {
        player_direction = v2_scale(player_direction, 1.0f / length);
    }

    bool state_set = false;

    // Wander if out of aggro range
    if (distance_to_player >= enemy -> enemy_logic.aggro_range)
    {
        enemy -> enemy_logic.enemy_state = ENEMYSTATE_patrol;
    }

    // Combat if within aggro range
    if (distance_to_player <= enemy -> enemy_logic.aggro_range)
    {
        enemy -> enemy_logic.enemy_state = ENEMYSTATE_combat;
    }

	// Flee if low HP and within aggro range
    if (distance_to_player <= enemy -> enemy_logic.aggro_range && enemy -> enemy_entity.health < (enemy -> enemy_entity.max_health / 4))
    {
        enemy -> enemy_logic.enemy_state = ENEMYSTATE_flee;
    }

	// Knockback state
	if (enemy -> enemy_logic.current_attack_cooldown > 0.95)
	{
		enemy -> enemy_logic.enemy_state = ENEMYSTATE_knockback;
	}

	update_enemy_movement(enemy);
}

// :Projectiles

void spawn_projectile(Ability *ability, Entity *source_entity, float speed, AnimationInfo *animation, float32 scale, float spawn_radius, float max_distance, float max_time_alive, bool nova) 
{
	int total_shots = 1;

    if (world -> active_projectiles < MAX_PROJECTILES)
    {
		if (is_upgrade_in_ability_upgrade_list(ability -> ability_upgrades, UPGRADEID_Multishot) == true)
		{
			total_shots = 1 + get_upgrade_in_ability_upgrade_list(ability -> ability_upgrades, UPGRADEID_Multishot) -> level;
			//log("Multishot level: %i", get_upgrade_in_ability_upgrade_list(ability -> ability_upgrades, UPGRADEID_Multishot) -> level);
			//log("Damage: %i", ability -> damage);
			//log("Firebolt Level: %i", ability -> current_level);
		}

        for (int shot_index = 0; shot_index < total_shots; shot_index++) 
        {
            for (int i = 0; i < world -> active_projectiles + 1; i++) 
            {
                if (world -> projectiles[i].is_active == false) 
                {
                    world -> active_projectiles++;
                    Projectile *projectile = & world -> projectiles[i];
                    projectile -> is_active = true;
                    projectile -> speed = speed;
                    projectile -> damage = ability -> damage;
                    projectile -> animation = *animation;
                    projectile -> scale = scale;
                    projectile -> source_entity = source_entity;
                    projectile -> distance_traveled = 0.0f;
                    projectile -> max_distance = max_distance;
                    projectile -> time_alive = 0.0f;
                    projectile -> max_time_alive = max_time_alive;

                    // Calculate the player's center position
                    SpriteData sprite_data = sprites[source_entity -> sprite_ID];
                    Vector2 player_center = v2((source_entity -> pos.x + (sprite_data.image -> width * 0.5f)), (source_entity -> pos.y + (sprite_data.image -> height * 0.5f)));

                    Vector2 mouse_pos = get_mouse_pos_in_current_space();

                    // Calculate direction from player to mouse
                    Vector2 direction = v2_sub(mouse_pos, player_center);
                    float32 length = v2_length(direction);

                    Vector2 normalized_direction = direction;
                    if (length != 0.0f)
                    {
                        normalized_direction = v2_scale(direction, 1.0f / length);
                    }

                    // Base angle (toward the mouse) when nova is off
                    float base_angle = atan2f(normalized_direction.y, normalized_direction.x);

                    // Calculate angle for the current projectile
                    float current_angle;

					if (nova)
					{
						// Spread projectiles in a full 360-degree nova
						float angle_offset = (2 * PI32) / total_shots;
						current_angle = base_angle + (shot_index * angle_offset);
					}
					else
					{
						if (total_shots == 1)
						{
							// If only one shot, fire directly at the target (base angle)
							current_angle = base_angle;
						}
						else
						{
							// Spread projectiles in a cone towards the mouse
							float cone_angle = PI32 / 6.0f; // 30-degree cone
							if (total_shots > 1)
							{
								float angle_offset = cone_angle / (total_shots - 1); // Space within cone
								current_angle = base_angle - (cone_angle / 2) + (shot_index * angle_offset);
							}
							else
							{
								// Fallback in case of invalid shot count
								current_angle = base_angle;
							}
						}
					}

                    // Spawn the projectile at the edge of the spawn_radius circle
                    Vector2 spawn_position = v2_add(player_center, v2(spawn_radius * cosf(current_angle), spawn_radius * sinf(current_angle)));

                    projectile -> position = spawn_position;
					// Debug logging

					// printf("Player Center: (%f, %f)\n", player_center.x, player_center.y);
					// printf("Spawn Position: (%f, %f)\n", spawn_position.x, spawn_position.y);
					// printf("Mouse Position: (%f, %f)\n", mouse_pos.x, mouse_pos.y);
					// printf("Projectile Direction Angle: %f\n", angle);
					// printf("Spawn Radius: %f\n", spawn_radius);

					// Draw the debug circle around the player when projectile is cast
					//start_debug_circle(& circle_state, player_center, spawn_radius, 1.0);


                    if (length != 0.0f)
                    {
                        Vector2 velocity_direction = v2(cosf(current_angle), sinf(current_angle)); 
                        projectile -> velocity = v2_scale(velocity_direction, speed);

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
    }
}

void update_projectile(Projectile *projectile) 
{
    if (!projectile -> is_active) return;

    projectile -> time_alive += delta_t;

    Vector2 movement = v2_scale(projectile -> velocity, delta_t);

    // Accumulate the movement in the remainder variables
    projectile -> x_remainder += movement.x;
    projectile -> y_remainder += movement.y;

    // Calculate how much to move this frame (rounded to the nearest pixel)
    int moveX = roundf(projectile -> x_remainder);
    int moveY = roundf(projectile -> y_remainder);

    if (moveX != 0) 
    {
        projectile -> position.x += moveX;
        projectile -> x_remainder -= moveX;
    }

    if (moveY != 0) 
    {
        projectile -> position.y += moveY;
        projectile -> y_remainder -= moveY;
    }

    projectile -> distance_traveled += v2_length(movement);

    // Check if the projectile hits any entity
    Entity *hit_entity = projectile_collides_with_entity(projectile);
    if (hit_entity) 
    {
        damage_entity(hit_entity, projectile -> damage);

        // Deactivate the projectile after it hits an entity
        projectile -> is_active = false;
		world -> active_projectiles--;
        return;
    }

    // Check if the projectile should expire
    if (projectile -> distance_traveled >= projectile -> max_distance || projectile -> time_alive >= projectile -> max_time_alive) 
    {
        projectile -> is_active = false;
		world -> active_projectiles--;
        return;
    }

    update_animation(& projectile -> animation, & projectile -> position, projectile -> scale, & projectile -> rotation);
}

// :Rendering

// :UI Rendering

void set_screen_space() 
{
	current_draw_frame -> projection = world_frame.screen_proj;
	current_draw_frame -> camera_xform = world_frame.screen_view;
}
void set_world_space() 
{
	current_draw_frame -> projection = world_frame.world_proj;
	current_draw_frame -> camera_xform = world_frame.world_view;
}

// :UI

void draw_ui_box_frame(Matrix4 base_transform, Vector2 box_size, float border_thickness, Vector4 bg_color, Vector4 border_color, Draw_Frame *frame) 
{
    // Draw top border
    Vector2 top_border_size = v2(box_size.x, border_thickness);
    Matrix4 top_xform = m4_translate(base_transform, v3(0, box_size.y - (border_thickness / 2.0f), 0));
    draw_rect_xform_in_frame(top_xform, top_border_size, border_color, frame);

    // Draw bottom border
    Vector2 bottom_border_size = v2(box_size.x, border_thickness);
    Matrix4 bottom_xform = m4_translate(base_transform, v3(0, (-border_thickness / 2.0f), 0));
    draw_rect_xform_in_frame(bottom_xform, bottom_border_size, border_color, frame);

    // Draw left border
    Vector2 left_border_size = v2(border_thickness, box_size.y);
    Matrix4 left_xform = m4_translate(base_transform, v3( (-border_thickness / 2.0f), 0, 0));
    draw_rect_xform_in_frame(left_xform, left_border_size, border_color, frame);

    // Draw right border
    Vector2 right_border_size = v2(border_thickness, box_size.y);
    Matrix4 right_xform = m4_translate(base_transform, v3(box_size.x - (border_thickness / 2.0f), 0, 0));
    draw_rect_xform_in_frame(right_xform, right_border_size, border_color, frame);
}

void draw_resource_bar(Resource *resource, float y_pos, int icon_row_count, Vector4 color, Vector4 bg_color, Draw_Frame *frame)
{
	// Increment resource
	if (resource -> current < resource -> max)
	{
		resource -> current += resource -> per_second * delta_t;
	}

	// Resource Overflow Check
	if (resource -> current >= resource -> max)
	{
		resource -> current = resource -> max;
	}

	// Resource underflow check
	if (resource -> current <= 0)
	{
		resource -> current = 0;
	}

	//log("%f %f %f", current_resource, max_resource, resource_per_second);

	float bar_width = icon_size * icon_row_count;

	float x_start_pos = (screen_width * 0.025);

	int current_resource_int = (int)resource -> current;

	int max_resource_int = (int)resource -> max;

	float percentage_of_bar_width = (bar_width / 100.0);

	float current_resource_percentage = (resource -> current / resource -> max) * 100.0f;

	float bar_visual_size = (percentage_of_bar_width * current_resource_percentage);

	// Black background box
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
		draw_rect_xform_in_frame(xform, v2(bar_width, icon_size), bg_color, frame);
	}

	// Bar Fill
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
		draw_rect_xform_in_frame(xform, v2(bar_visual_size, icon_size), color, frame);
		draw_ui_box_frame(xform, v2(bar_width, icon_size), 1, bg_color, COLOR_WHITE, frame);
	}	

	// Bar current resource display
	{
		string current_resource_string = STR("%s: %i/%i    +%.1f/s"); // %i is where the number goes.

		current_resource_string = sprint(get_temporary_allocator(), current_resource_string, resource -> name, current_resource_int, max_resource_int, resource -> per_second);

		Gfx_Text_Metrics metrics = measure_text(font, current_resource_string, font_height, v2(0.20, 0.20));

		Vector2 draw_pos = v2(x_start_pos + (bar_width * 0.5), y_pos + 14);

		draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
		
		draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

		draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

		draw_text_in_frame(font, current_resource_string, font_height, draw_pos, v2(0.20, 0.20), COLOR_WHITE, frame);
	}
}

void draw_unit_bar(Vector2 position, float *current_value, float *max_value, float *recovery_per_second, int icon_size, int icon_row_count, Vector4 color, Vector4 bg_color, Draw_Frame *frame)
{
	// Update bar
	if (*current_value < *max_value)
	{
		*current_value += *recovery_per_second * delta_t;
	}

	// Value overflow check
	if (*current_value >= *max_value)
	{
		*current_value = *max_value;
	}

	// Value underflow check
	if (*current_value <= 0)
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
		draw_rect_xform_in_frame(xform, v2(bar_width, icon_size), bg_color, frame);
	}

	// Bar Fill
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3((position.x - (bar_width * 0.5)), (position.y + icon_size), 0.0));
		draw_rect_xform_in_frame(xform, v2(bar_visual_size, icon_size), color, frame);
		draw_ui_box_frame(xform, v2(bar_visual_size, icon_size), 1, color, COLOR_WHITE, frame);
	}	
}

Draw_Quad* draw_button(string button_tooltip, Vector2 button_size, Vector2 button_position, Vector4 color, Draw_Frame *frame)
{
	Vector2 button_size_v2 = v2(16.0, 16.0);

	Matrix4 xform = m4_scalar(1.0);

	xform = m4_translate(xform, v3(button_position.x, button_position.y, 0.0)); 

	float border_thickness = 1;
	Draw_Quad* quad = draw_rect_xform_in_frame(xform, button_size, bg_box_color, frame);
	draw_ui_box_frame(xform, button_size, border_thickness, bg_box_color, COLOR_WHITE, frame);

	// Draw Button Text
	Gfx_Text_Metrics metrics = measure_text(font, button_tooltip, font_height, v2(0.1, 0.1));
	Vector2 draw_pos = v2((button_position.x + (button_size.x * 0.5)), (button_position.y + (button_size.y * 0.5)));
	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

	draw_text_in_frame(font, button_tooltip, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE, frame);

	return quad;
}

bool check_if_mouse_clicked_button(Vector2 button_pos, Vector2 button_size)
{
	Range2f button_range = range2f_make_bottom_left(button_pos, button_size);

	// Check if mouse is on button
	if (range2f_contains(button_range, get_mouse_pos_in_current_space())) 
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
	Range2f button_range = range2f_make_bottom_left(button_pos, button_size);

	// Check if mouse is on button
	if (range2f_contains(button_range, get_mouse_pos_in_current_space())) 
	{
		return true;
	}
	else
	{
		return false;
	}
}

void draw_tooltip_box_string_below_same_size(Draw_Quad* quad, float tooltip_size, string *title, Draw_Frame *frame)
{
	Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

	Range2f screen_range = quad_to_range(screen_quad);

	Vector2 icon_center = range2f_get_center(screen_range);

	Matrix4 xform = m4_scalar(1.0);

	Vector2 tooltip_box_size = v2(tooltip_size, tooltip_size);

	xform = m4_translate(xform, v3(tooltip_box_size.x * -0.5, - tooltip_box_size.y - tooltip_size * 0.5, 0));

	xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

    float border_thickness = 1;
    draw_rect_xform_in_frame(xform, tooltip_box_size, bg_box_color, frame);
	draw_ui_box_frame(xform, button_size, border_thickness, bg_box_color, COLOR_WHITE, frame);

	float current_y_pos = icon_center.y;

	// String Display	
	
	Gfx_Text_Metrics metrics = measure_text(font, *title, font_height, v2(0.1, 0.1));

	Vector2 draw_pos = icon_center;

	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	
	draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

	draw_pos = v2_add(draw_pos, v2(0, tooltip_size * -0.5));

	draw_pos = v2_add(draw_pos, v2(0, -2.0)); // Padding

	draw_text_in_frame(font, *title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE, frame);

	current_y_pos = draw_pos.y;
}

void draw_tooltip_box_string_to_side_larger_xform(Draw_Quad* quad, float tooltip_size, string *title, Draw_Frame *frame)
{
	Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

	Range2f screen_range = quad_to_range(screen_quad);

	Vector2 icon_center = range2f_get_center(screen_range);

	Matrix4 xform = m4_scalar(1.0);

	Vector2 tooltip_box_size = v2(tooltip_size * 4, tooltip_size * 2);

	xform = m4_translate(xform, v3((tooltip_box_size.x) + (tooltip_size * 4), (- tooltip_box_size.y) + (tooltip_size * 0.5), 0));

	xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

	draw_rect_xform_in_frame(xform, tooltip_box_size, bg_box_color, frame);

	// String Display	

	Matrix4 text_xform = m4_scalar(1.0);

	text_xform = m4_translate(xform, v3((tooltip_box_size.x * 0.10), (tooltip_box_size.y * 0.65), 0));
	
	draw_text_xform_in_frame(font, *title, font_height, text_xform, v2(0.1, 0.1), COLOR_WHITE, frame);
}

void draw_tooltip_box_string_to_side_larger(Draw_Quad* quad, Vector2 button_size, Vector2 tooltip_size, string *title, Draw_Frame *frame) 
{
    // Convert to screen space
    Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);
    Range2f screen_range = quad_to_range(screen_quad);
    Vector2 button_center = range2f_get_center(screen_range);

    // Create transformation matrix
    Matrix4 xform = m4_scalar(1.0f);

    // Set the position of the tooltip box relative to the icon center
    Vector2 button_position = v2((button_center.x + (tooltip_size.x / 2)), (button_center.y - (tooltip_size.y / 2)));

	//Vector2 button_position = v2(((screen_width / 2) - (tooltip_size.x / 2)), (screen_height - tooltip_size.y - button_size.y));

    // Translate the transformation matrix to the correct position
    xform = m4_translate(xform, v3(button_position.x, button_position.y, 0.0f));

    // Draw the tooltip background with border
    float border_thickness = 1;
    draw_rect_xform_in_frame(xform, tooltip_size, bg_box_color, frame);
	draw_ui_box_frame(xform, tooltip_size, border_thickness, bg_box_color, COLOR_WHITE, frame);

    // Text display inside the tooltip
    Gfx_Text_Metrics metrics = measure_text(font, *title, font_height, v2(0.1f, 0.1f));

    // Calculate position for drawing text centered in the tooltip box
    Vector2 draw_pos = v2_add(button_position, v2_mul(tooltip_size, v2(0.5f, 0.5f)));
    draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
    draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5f, 0.5f)));

    // Draw the text
    draw_text_in_frame(font, *title, font_height, draw_pos, v2(0.1f, 0.1f), COLOR_WHITE, frame);
}

void display_skill_level_up_button(Vector2 button_size, Vector4 color, Draw_Frame *frame)
{
	int y_pos = 220;
	int current_buttons = 0;

	for (int i = 0; i < SKILLID_MAX; i++)
	{
		if (get_player_skill(skills[i].skill_ID) != NULL && get_player_skill(skills[i].skill_ID) -> unlocked == true)
		{
			current_buttons++;
			Vector2 button_pos = v2((screen_width * 0.025), (y_pos - (current_buttons * 20)));
			Skill* skill = get_player_skill(skills[i].skill_ID);

			// Build the costs string
			string costs_text = STR("");
			for (int j = 0; j < MAX_COSTS; j++) 
			{
				if (skill -> current_costs[j] > 0) // Only add non-zero costs
				{
					if (j > 0) 
					{
						costs_text = string_concat(costs_text, STR(", "), get_temporary_allocator());
					}
					costs_text = string_concat(costs_text, sprint(get_temporary_allocator(), STR("%.1f %s"), skill -> current_costs[j], get_player_resource(skill -> cost_resources[j]) -> name), get_temporary_allocator());
				}
			}

			string button_text = sprint(get_temporary_allocator(), STR("%s\nLevel: %i\nCost: %s"), skill -> name, skill -> level, costs_text);
			string button_tooltip = sprint(get_temporary_allocator(), STR("%s\nLevel: %i\nCost: %s\n+%.2f %s \n%s"), skill -> name, skill -> level, costs_text, skill -> current_effect_value, skill -> effect_text, skill -> description);

			if (check_if_mouse_clicked_button(button_pos, button_size) == true)
			{
				world_frame.hover_consumed = true;
				find_skill_to_level(skills[i].skill_ID);
			}

			Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color, frame);

			if (check_if_mouse_hovering_button(button_pos, button_size) == true)
			{
				world_frame.hover_consumed = true;
				quad -> color = COLOR_RED;
				draw_tooltip_box_string_to_side_larger(quad, button_size, tooltip_size, & button_tooltip, frame);
			}
		}
	}
}

void unlock_upgrade(UpgradeID upgrade_ID)
{
	if (get_player_upgrade(upgrade_ID) == NULL)
	{
		mark_upgrade_unlocked(upgrade_ID);
		update_known_upgrades();
		add_upgrade_to_player(upgrade_ID);

		for (int i = 0; i < RESOURCEID_MAX; i++)
		{ 
			if (get_player_upgrade(upgrade_ID) -> resources_unlocked[i] != RESOURCEID_nil)
			{
				add_player_resource(get_player_upgrade(upgrade_ID) -> resources_unlocked[i]);
			}
		}

		for (int i = 0; i < SKILLID_MAX; i++)
		{
			if (get_player_upgrade(upgrade_ID) -> skills_unlocked[i] != RESOURCEID_nil)
			{
				add_player_skill(get_player_upgrade(upgrade_ID) -> skills_unlocked[i]);
			}
		}

		for (int i = 0; i < ABILITYID_MAX; i++)
		{
			if (get_player_upgrade(upgrade_ID) -> abilities_unlocked[i] != ABILITYID_Nil)
			{
				add_player_ability(get_player_upgrade(upgrade_ID) -> abilities_unlocked[i]);
			}
		}

		for (int i = 0; i < UPGRADEID_MAX; i++)
		{
			if (get_player_upgrade(upgrade_ID) -> upgrades_revealed[i] != UPGRADEID_nil)
			{
				mark_upgrade_known(get_player_upgrade(upgrade_ID) -> upgrades_revealed[i]);
			}
		}
	}
}

void display_upgrade_buttons(Vector2 button_size, Vector4 color, Draw_Frame *frame)
{
	update_known_upgrades();

	int y_pos = 260;
	
	int current_buttons = 0;

	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (is_upgrade_in_known(world -> player.all_upgrades[i].upgrade_ID) == true)
		{
			if (get_player_upgrade(world -> player.all_upgrades[i].upgrade_ID) == NULL)
			{
				current_buttons++;

				Vector2 button_pos = v2(300, (y_pos - (current_buttons * 20)));

				string button_text = sprint(get_temporary_allocator(), STR(world -> player.all_upgrades[i].name));

				string button_tooltip =  sprint(get_temporary_allocator(), STR(world -> player.all_upgrades[i].description));

				Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color, frame);	

				if (check_if_mouse_clicked_button(button_pos, button_size) == true)
				{
					world_frame.hover_consumed = true;

					unlock_upgrade(world -> player.all_upgrades[i].upgrade_ID);
				}

				if (check_if_mouse_hovering_button(button_pos, button_size) == true)
				{
					world_frame.hover_consumed = true;
					quad -> color = COLOR_RED;
					draw_tooltip_box_string_to_side_larger(quad, button_size, tooltip_size, & button_tooltip, frame);
				}					
			}
		}
	}
}

void display_ability_upgrade_buttons(Vector2 button_size, Vector4 color, Draw_Frame *frame)
{
	int y_pos = 160;
	
	int current_buttons = 0;

	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (get_player_upgrade(world -> player.all_upgrades[i].upgrade_ID) != UPGRADEID_nil && get_player_upgrade(world -> player.all_upgrades[i].upgrade_ID) -> has_levels == true)
		{
			current_buttons++;

			Ability* ability = get_player_ability(world -> player.all_upgrades[i].abilities_unlocked[0]);

			Upgrade* upgrade = get_player_upgrade(world -> player.all_upgrades[i].upgrade_ID);

			Vector2 button_pos = v2((screen_width * 0.025), (y_pos - (current_buttons * 20)));

			string button_text = sprint(get_temporary_allocator(), STR("Level Up: %s \n Current Level: %i"), upgrade -> level_up_text, upgrade -> level);

			string button_tooltip;

			if (ability -> ability_ID != ABILITYID_Nil)
			{
				button_tooltip = sprint(get_temporary_allocator(), STR("%s\nLevel: %i \nDamage: %i \nMana Cost: %i"), ability -> name, ability -> current_level, ability -> damage, ability -> resource_cost);
			}
			else
			{
				button_tooltip = sprint(get_temporary_allocator(), STR("%s\nLevel: %i"), upgrade -> level_up_text, upgrade -> level);
			}

			Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color, frame);	

			if (check_if_mouse_clicked_button(button_pos, button_size) == true)
			{
				world_frame.hover_consumed = true;
				level_up_upgrade(world -> player.all_upgrades[i].upgrade_ID);
			}

			if (check_if_mouse_hovering_button(button_pos, button_size) == true)
			{
				world_frame.hover_consumed = true;
				quad -> color = COLOR_RED;
				draw_tooltip_box_string_to_side_larger(quad, button_size, tooltip_size, & button_tooltip, frame);
			}					
		}
	}
}

void render_ui(Draw_Frame *frame)
{
	set_screen_space();

	push_z_layer_in_frame(Layer_UI, current_draw_frame);

	Vector2 txt_scale = v2(0.1, 0.1);
	Vector4 bg_col = v4(0, 0, 0, 0.90);
	Vector4 fill_col = v4(0.5, 0.5, 0.5, 1.0);
	Vector4 accent_col_blue = hex_to_rgba(0x44c3daff);
	Vector4 accent_col_purple = hex_to_rgba(0x9d00ffff);

	// :Fps Display

	string current_fps = tprint("%i FPS %.2f ms", fps_display, frame_time_display);

	draw_text_with_pivot(font, current_fps, font_height, v2(5, 265), v2(0.15, 0.15), COLOR_WHITE, PIVOT_top_left, frame);

	// :Inventory UI
	{
		if (is_key_just_pressed(KEY_TAB))
		{
			consume_key_just_pressed(KEY_TAB);

			// Swap to Inventory UI Open State.
			world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);
		}

		world -> inventory_alpha_target = (world -> ux_state == UX_inventory ? 1.0 : 0.0);
		animate_f32_to_target(& world -> inventory_alpha, world -> inventory_alpha_target, delta_t, 200.0); //speed inventory closes / fades
		bool is_inventory_enabled = world -> inventory_alpha_target == 1;

		if (world -> inventory_alpha != 0.0)
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

			const int icon_row_count = 8;

			const int icon_row_count_items = 3;

			float entire_thing_width = icon_row_count_items * icon_size;
			float x_start_pos = (screen_width * 0.5) - (entire_thing_width * 0.5);

			// turning inventory off because it's causing crashes and i don't know why.
			/*

			// Black background box
			{
				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
				draw_rect_xform(xform, v2(entire_thing_width, icon_size), bg_box_color);
			}

			int slot_index = 0;
			for (int i = 1; i < ITEM_MAX; i++)
			{
				ItemData* item = & world -> items[i];

				if (item -> amount > 0)
				{
					// Draw item icons
					float slot_index_offset = slot_index * icon_size;

					Matrix4 xform = m4_scalar(1.0);

					xform = m4_translate(xform, v3(x_start_pos + slot_index_offset, y_pos, 0.0));
					SpriteData sprite_data = item -> sprite_data;
					Vector2 icon_positon = v2(x_start_pos + slot_index_offset, y_pos);
					
					// White transparent box to show item slot is filled.
					Draw_Quad* quad = draw_rect_xform(xform, v2(icon_size, icon_size), v4(1, 1, 1, 0.2));

					Range2f icon_box = quad_to_range(*quad);

					float is_selected_alpha = 0.0;
				
					if (is_inventory_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc()))
					{
						is_selected_alpha = 1.0;
					}

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

					xform = m4_translate(xform, v3(icon_size * -0.5,  icon_size * -0.5, 0));

					// Draw Sprite

					Range2f box = range2f_make_bottom_left(icon_positon, v2(icon_size, icon_size));

					//draw_image_xform(sprite -> image, xform, v2(icon_size, icon_size), COLOR_WHITE); // old

					draw_sprite_in_rect(sprite_data, box, COLOR_WHITE, 0.2); // New sprite rendering from randy day 11
				
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
			*/

			// Mana bar
			if (get_player_resource(RESOURCEID_Mana) != NULL && get_player_resource(RESOURCEID_Mana) -> unlocked == true)
			{
				draw_resource_bar(get_player_resource(RESOURCEID_Mana), 240, icon_row_count, accent_col_blue, bg_box_color, frame);
			}

			// Intellect bar
			if (get_player_resource(RESOURCEID_Intellect) != NULL && get_player_resource(RESOURCEID_Intellect) -> unlocked == true)
			{
				draw_resource_bar(get_player_resource(RESOURCEID_Intellect), 220, icon_row_count, accent_col_purple, bg_box_color, frame);
			}

			display_skill_level_up_button(button_size, fill_col, frame);
			display_upgrade_buttons(button_size, fill_col, frame);
			display_ability_upgrade_buttons(button_size, fill_col, frame);
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
	pop_z_layer_in_frame(current_draw_frame);
}

// :Floor Rendering

void render_floor_tiles(Draw_Frame *frame)
{	
	FloorData* floor = & world -> floors[world -> current_floor];

	float half_tile_width = tile_width * 0.5f;

	for (int i = 0; i < MAX_TILE_COUNT; i++) 
	{
		TileData* tile_data = & floor -> tiles[i];

		if (tile_data -> is_valid == true)
		{
			int x = tile_data -> tile.x;
			int y = tile_data -> tile.y;

			// Checkerboard color pattern
			Vector4 col = color_0;
			//log("%f, %f, %f, %f", color_0.r, color_0.g, color_0.b, color_0.a);

			if (((x + y) % 2) == 0) 
			{
				col.a = 0.8;
			}
			
			float x_pos = x * tile_width;
			float y_pos = y * tile_width;
			
			draw_rect_in_frame(v2(x_pos - half_tile_width, y_pos - half_tile_width), v2(tile_width, tile_width), col, frame);
		}
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

// :Building Rendering

void render_buildings(Draw_Frame *frame)
{
	FloorData* floor = & world -> floors[world -> current_floor];

	float half_tile_width = tile_width * 0.5f;

	for (int i = 0; i < MAX_TILE_COUNT; i++) 
	{
		TileData* tile_data = & floor -> tiles[i];
		
		int x = tile_data -> tile.x;
		int y = tile_data -> tile.y;

		float x_pos = x * tile_width;
		float y_pos = y * tile_width;

		if (tile_data -> building.building_ID != 0)
		{
			SpriteData sprite_data = tile_data -> building.sprite_data;

			Vector2 sprite_size = get_sprite_size(sprite_data);

			Matrix4 xform = m4_scalar(1.0);

			xform = m4_translate(xform, v3(x_pos - half_tile_width, y_pos - half_tile_width, 0));

			draw_image_xform_in_frame(sprites[tile_data -> building.sprite_data.sprite_ID].image, xform, sprite_size, COLOR_WHITE, frame);

			// pos debug
			//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), tile_data -> building.pos.x, tile_data -> building.pos.y), font_height, tile_data -> building.pos, v2(0.2, 0.2), COLOR_WHITE);
			
			// floor debug
			//draw_text(font, sprint(get_temporary_allocator(), STR("Current Floor:%i"), tile_data -> building.current_floor), font_height, tile_data -> building.pos, v2(0.2, 0.2), COLOR_WHITE);
		}
	}
}

// :Player Rendering

void render_player(Draw_Frame *frame)
{
	Entity *player = get_player();
	SpriteData sprite_data = sprites[player -> sprite_ID];

	Vector2 sprite_size = get_sprite_size(sprite_data);

	Matrix4 xform = m4_scalar(1.0);

	xform = m4_translate(xform, v3(player -> pos.x, player -> pos.y, 0));

	Vector4 col = COLOR_WHITE;
	draw_image_xform_in_frame(sprite_data.image, xform, sprite_size, col, frame);

	// Healthbar test values
	Vector2 health_bar_pos = v2((player -> pos.x + (sprite_data.image -> width * 0.5)), (player -> pos.y + (sprite_data.image -> height)));

	// Temperary render player healthbar test
	draw_unit_bar(health_bar_pos, & player -> health, & player -> max_health, & player -> health_regen, 4, 6, COLOR_RED, bg_box_color, frame);

	// World space current location debug for object pos
	//draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), player -> pos.x, player -> pos.y), font_height, player -> pos, v2(0.2, 0.2), COLOR_WHITE);

	// floor player currently resides in
	//draw_text(font, sprint(get_temporary_allocator(), STR("Current Floor:%i"), player -> current_floor), font_height, player -> pos, v2(0.2, 0.2), COLOR_WHITE);

	// Draw (REAL) Player pos as blue pixel
	//draw_rect(v2(player -> pos.x, player -> pos.y), v2(1, 1), v4(0, 255, 255, 1));

	// Draw player pos offset to be centered on the center of the sprite (NOT REAL POS)
	//draw_rect(v2((player -> pos.x + sprites[player -> spriteID].image -> width * 0.5), (player -> pos.y + sprites[player -> spriteID].image -> height * 0.5)), v2(1, 1), v4(0, 255, 0, 1));
}

// Entities Rendering

void render_entities(Draw_Frame *frame)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; i++)
	{
		Entity* en = & world -> floors[world -> current_floor].enemies[i].enemy_entity;
		//log("%i floor in rendering",  world -> current_floor);

		if (en -> is_valid)
		{
			switch (en -> entity_ID)
			{	
				case ENTITY_Player:
				{
					break;
				}

				default:
				{
					// Get sprite dimensions
					Vector2 sprite_size = get_sprite_size(sprites[en -> sprite_ID]);

					Matrix4 xform = m4_scalar(1.0);

					/*
					if(en -> is_item == true)
					{
						xform = m4_translate(xform, v3(0, 2 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0));
					}
					*/

					xform = m4_translate(xform, v3(en -> pos.x, en -> pos.y, 0));

					Vector4 col = COLOR_WHITE;

					draw_image_xform_in_frame(sprites[en -> sprite_ID].image, xform, sprite_size, col, frame);

					Vector2 health_bar_pos = v2((en -> pos.x + (sprites[en -> sprite_ID].image -> width * 0.5)), (en -> pos.y + (sprites[en -> sprite_ID].image -> height)));

					// Temp healthbar for non-players
					draw_unit_bar(health_bar_pos, & en -> health, & en -> max_health, & en -> health_regen, 4, 6, COLOR_RED, bg_box_color, frame);

					//world space current location debug for object pos
					//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), en -> pos.x, en -> pos.y), font_height, en -> pos, v2(0.1, 0.1), COLOR_WHITE);
								
					// floor debug
					//draw_text(font, sprint(get_temporary_allocator(), STR("Current Floor:%i"), en -> current_floor), font_height, en -> pos, v2(0.2, 0.2), COLOR_WHITE);
					
					break;
				}
			}
		}
	}
}

Gfx_Shader_Extension load_shader(string file_path, int cbuffer_size) 
{
	string source;
	
	bool ok = os_read_entire_file(file_path, & source, get_heap_allocator());
	assert(ok, "Could not read %s", file_path);
	
	Gfx_Shader_Extension shader;
	ok = gfx_compile_shader_extension(source, cbuffer_size, & shader);
	assert(ok, "Failed compiling shader extension");
	
	return shader;
}

bool button(string label, Vector2 pos, Vector2 size, bool enabled) 
{
	Vector4 color = v4(.45, .45, .45, 1);
	
	float L = pos.x;
	float R = L + size.x;
	float B = pos.y;
	float T = B + size.y;
	
	float mx = input_frame.mouse_x - window.width / 2;
	float my = input_frame.mouse_y - window.height / 2;

	bool pressed = false;

	if (mx >= L && mx < R && my >= B && my < T) 
	{
		color = v4(.15, .15, .15, 1);
		if (is_key_down(MOUSE_BUTTON_LEFT)) 
		{
			color = v4(.05, .05, .05, 1);
		}
		
		pressed = is_key_just_released(MOUSE_BUTTON_LEFT);
	}
	
	if (enabled) 
	{
		color = v4_sub(color, v4(.2, .2, .2, 0));
	}

	draw_rect(pos, size, color);
	
	Gfx_Text_Metrics m = measure_text(font, label, font_height, v2(1, 1));
	
	Vector2 bottom_left = v2_sub(pos, m.functional_pos_min);
	bottom_left.x += size.x / 2;
	bottom_left.x -= m.functional_size.x / 2;
	
	bottom_left.y += size.y / 2;
	bottom_left.y -= m.functional_size.y / 2;
	
	draw_text(font, label, font_height, bottom_left, v2(1, 1), COLOR_WHITE);
	
	return pressed;
}

string view_mode_stringify(View_Mode vm) 
{
	switch (vm) {
		case VIEW_GAME_AFTER_POSTPROCESS:
			return STR("VIEW_GAME_AFTER_POSTPROCESS");
		case VIEW_GAME_BEFORE_POSTPROCESS:
			return STR("VIEW_GAME_BEFORE_POSTPROCESS");
		case VIEW_BLOOM_MAP:
			return STR("VIEW_BLOOM_MAP");
		default: return STR("");
	}
}

void draw_game(Draw_Frame *frame) 
{
	// Draw a background
	draw_rect_in_frame(v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), v4(.2, .2, .2, 1), frame);

	tm_scope("Render Floor Tiles")
	{
		render_floor_tiles(frame);
	}

	tm_scope("Render Buildings")
	{
		render_buildings(frame);
	}

	tm_scope("Render entities")
	{
		render_entities(frame);
	}

	tm_scope("Render Player")
	{
		render_player(frame);
	}
}

void draw_ui(Draw_Frame *frame)
{
	tm_scope("Render UI")
	{
		render_ui(frame);
	}
}

// :World setup

void world_setup()
{
    log("fresh world made");
    
    // Start inventory open
    world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);

    if (world -> current_floor == null)
    {
        world -> current_floor = 0;
    }
    
    log("%i current floor", world -> current_floor);
    
    world -> floors[world -> current_floor] = *create_empty_floor(true, world -> current_floor);
    
    world -> active_floors++;

    setup_player(& world -> player);
    memcpy(world -> player.all_upgrades, upgrades, sizeof(upgrades));

    // :test stuff
    #if defined(DEV_TESTING)
    {
        world -> items[ITEM_Exp] = setup_exp_item();

        // Spawn one Target
        Entity* en = entity_create();
        setup_target(en);
    }
    #endif
}

// :Save / Load System

bool create_directory_if_not_exists(const char* path) 
{
    DWORD ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES) 
	{
        // Path does not exist, create it
        if (CreateDirectoryA(path, NULL)) 
		{
            return true;
        
		} 
		else 
		{
            return false; 
        }
    }
    return (ftyp & FILE_ATTRIBUTE_DIRECTORY) != 0; // Path exists
}

string get_saves_path() 
{
    char exe_path[260];
    get_executable_path(exe_path, sizeof(exe_path));

    // Create the "Saves" folder path
    String_Builder path_builder;
    string_builder_init(& path_builder, get_temporary_allocator());

    // Append executable path
    string_builder_append(& path_builder, STR(exe_path));
    string_builder_append(& path_builder, STR("\\Saves"));

    // Create the directory if it doesn't exist
    create_directory_if_not_exists((const char*)path_builder.buffer);

    return string_builder_get_string(path_builder);
}

bool world_save_to_disk() 
{
    string saves_path = get_saves_path();

    // Construct the full path for the world file
    String_Builder world_path_builder;
    string_builder_init(& world_path_builder, get_temporary_allocator());
    
    string_builder_append(& world_path_builder, saves_path);
    string_builder_append(& world_path_builder, STR("\\world"));

    u64 everything_but_floors = offsetof(World, floors);
    u64 active_floors = world -> active_floors * sizeof(FloorData);
    u64 all_save_data = everything_but_floors + active_floors;

    return os_write_entire_file_s(string_builder_get_string(world_path_builder), (string){all_save_data, (u8*)world});
}

bool world_attempt_load_from_disk() 
{
    string saves_path = get_saves_path();

    // Construct the full path for the world file
    String_Builder world_path_builder;
    string_builder_init(& world_path_builder, get_temporary_allocator());
    
    string_builder_append(& world_path_builder, saves_path);
    string_builder_append(& world_path_builder, STR("\\world"));

    string result = {0};
    
	bool succ = os_read_entire_file_s(string_builder_get_string(world_path_builder), & result, get_heap_allocator()); // <- Allocate on heap instead (temp is killed every frame)
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

	/*
    if (result.count != sizeof(World)) 
    {
        log_error("world size different to one on disk.");
        return false;
    }
	*/

    memcpy(world, result.data, result.count);
    dealloc_string(get_heap_allocator(), result); 

	// After loading the skills, restore the function pointers
    for (int i = 0; i < SKILLID_MAX; i++) 
    {
        set_skill_functions(& world -> player.skill_list[i]);
    }
	
	// After loading the abilties, restore the function pointers
    for (int i = 0; i < ABILITYID_MAX; i++) 
    {
        set_ability_functions(& world -> player.ability_list[i]);
    }

    return true;
}

Matrix4 construct_view_matrix(Vector2 pos, float zoom) 
{
	Matrix4 view = m4_identity;

	view = m4_identity;

	// randy: these might be ordered incorrectly for the camera shake. Not sure.

	// translate into position
	view = m4_translate(view, v3(pos.x, pos.y, 0));

	// scale the zoom
	view = m4_scale(view, v3(1.0 / zoom, 1.0 / zoom, 1.0));

	return view;
}

void set_world_view() 
{
	world_frame.world_view = construct_view_matrix(camera_pos, camera_zoom);
	world_frame.camera_pos_copy = camera_pos;
}

// :Game Program Setup

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
	window.force_topmost = false;

	world = alloc(get_heap_allocator(), sizeof(World));

	last_time = os_get_elapsed_seconds();

	// :Color

	color_0 = hex_to_rgba(0x2f5ebdff);

    char exe_path_char[260];
    
    get_executable_path(exe_path_char, sizeof(exe_path_char)); 
    string exe_path = char_to_string(exe_path_char);

    load_sprite_data(exe_path);
	load_resource_data();
	load_skill_data();
	load_ability_data();
	load_upgrade_data();
	load_enemy_defaults();

	setup_fireball_anim(exe_path); // Setup fireball animation so it can be used.

	// :Font Setup

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());

	assert(font, "Failed loading arial.ttf, %d", GetLastError());

	// :Camera Settings

	string saves_path = get_saves_path();

	String_Builder world_path_builder;
	string_builder_init(& world_path_builder, get_temporary_allocator());

	string_builder_append(& world_path_builder, saves_path);
	string_builder_append(& world_path_builder, STR("\\world"));

	string world_path = string_builder_get_string(world_path_builder);

	if (os_is_file_s(world_path)) 
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

	// :Shader Stuff

	// regular shader + point light which makes things extra bright
	Gfx_Shader_Extension light_shader = load_shader(STR("oogabooga/examples/bloom_light.hlsl"), sizeof(Scene_Cbuffer));
	
	// shader used to generate bloom map. Very simple: It takes the output color -1 on all channels 
	// so all we have left is how much bloom there should be
	Gfx_Shader_Extension bloom_map_shader = load_shader(STR("oogabooga/examples/bloom_map.hlsl"), sizeof(Scene_Cbuffer));
	
	// postprocess shader where the bloom happens. It samples from the generated bloom_map.
	Gfx_Shader_Extension postprocess_bloom_shader = load_shader(STR("oogabooga/examples/bloom.hlsl"), sizeof(Scene_Cbuffer));
	
	Gfx_Image *bloom_map = 0;
	Gfx_Image *game_image = 0;
	Gfx_Image *final_image = 0;
	Gfx_Image *ui_image = 0;
	
	View_Mode view = VIEW_GAME_AFTER_POSTPROCESS;
	
	Draw_Frame offscreen_draw_frame;
	draw_frame_init(& offscreen_draw_frame);

	Draw_Frame ui_draw_frame;
	draw_frame_init(& ui_draw_frame);
	
	Scene_Cbuffer scene_cbuffer;

	// Window width and height may be 0 before first call to os_update(), and we base render target sizes of window size.
	// This is an Oogabooga quirk which might get fixed at some point.
	os_update();

	// :Game Loop

	string current_fps = STR("0");

	while (!window.should_close) 
	{
		reset_temporary_storage();
		world_frame = (WorldFrame){0};
		current_draw_frame = 0;
		current_draw_frame = & offscreen_draw_frame;
		
		// :Time tracking
		current_time = os_get_elapsed_seconds();
		delta_t = current_time - last_time;

		calculate_fps();

		last_time = current_time;
						
		// :Projection & Sorting

		draw_frame.enable_z_sorting = true;

		world_frame.world_proj = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		// Camera
		{
			Vector2 target_pos = get_player() -> pos;
			animate_v2_to_target(& camera_pos, target_pos, delta_t, 15.0f);

			set_world_view();
		}

		world_frame.screen_proj = m4_make_orthographic_projection(0.0, screen_width, 0.0, screen_height, -1, 10);
		world_frame.screen_view = m4_identity;

		world_frame.render_target_h = window.height;
		world_frame.render_target_w = window.width;

		set_world_space();

		push_z_layer_in_frame(Layer_WORLD, current_draw_frame);

		Vector2 mouse_pos_world = get_mouse_pos_in_current_space();
		int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
		int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);

		// :World update
		{
			world -> time_elapsed += delta_t;
		}

		update_cooldown(& world -> floor_cooldown);

		// Debug Visuals
		//update_debug_circle(& circle_state);
	
		tm_scope("Update Projectiles")
		{
			// Loop through all Projectiles and render / move them.
			for (int i = 0; i < MAX_PROJECTILES; i++) 
			{		
				if (world -> projectiles[i].is_active) 
				{
					update_projectile(& world -> projectiles[i]);
				}
			}
		}

		tm_scope("Update enemies")
		{
			// Loop through all enemies and update enemy state
			for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
			{	
				if (world -> floors[world -> current_floor].enemies[i].enemy_entity.is_valid == true) 
				{
					update_enemy_states(& world -> floors[world -> current_floor].enemies[i]);
				}
			}
		}

		if (is_key_just_pressed(KEY_F7))
		{
			load_previous_floor();
		}
		
		if (is_key_just_pressed(KEY_F8))
		{
			load_next_floor();
		}

		if (is_key_just_pressed('1'))
		{
			// Draw the debug circle around the player
			// start_debug_circle(& circle_state, player -> pos, 80, 0.5);

			tm_scope("Spawn Projectile")
			{
				if(get_player_resource(RESOURCEID_Mana) != NULL)
				{
					if (is_ability_in_player_list(ABILITYID_Fire_Bolt) == true)
					{
						if (get_player_resource(RESOURCEID_Mana) -> current >= get_player_ability(ABILITYID_Fire_Bolt) -> resource_cost)
						{
							spawn_projectile(get_player_ability(ABILITYID_Fire_Bolt), get_player(), 250.0, & Fireball, 1.0, 22, 1000, 5, false);

							get_player_resource(RESOURCEID_Mana) -> current -= get_player_ability(ABILITYID_Fire_Bolt) -> resource_cost;
						}
					}
				}
			}
		}

		if (is_key_just_pressed('2'))
		{
			// Draw the debug circle around the player
			// start_debug_circle(& circle_state, player -> pos, 80, 0.5);

			tm_scope("Spawn Projectile")
			{
				if(get_player_resource(RESOURCEID_Mana) != NULL)
				{
					if (is_ability_in_player_list(ABILITYID_Fire_Bolt) == true)
					{
						if (get_player_resource(RESOURCEID_Mana) -> current >= get_player_ability(ABILITYID_Fire_Bolt) -> resource_cost)
						{
							spawn_projectile(get_player_ability(ABILITYID_Fire_Bolt), get_player(), 250.0, & Fireball, 1.0, 22, 1000, 5, true);

							get_player_resource(RESOURCEID_Mana) -> current -= get_player_ability(ABILITYID_Fire_Bolt) -> resource_cost;
						}
					}
				}
			}
		}

		// Press F1 to Close Game
		if (is_key_just_pressed(KEY_F1))
		{
			window.should_close = true;
		}

		// Camera zoom
		if (is_key_down(KEY_SHIFT)) 
		{
			if (is_key_down('Q')) 
			{
				camera_zoom += 1 * delta_t;
				camera_zoom = clamp_top(camera_zoom, 4);
			}
			if (is_key_down('E')) 
			{
				camera_zoom -= 1 * delta_t;
				camera_zoom = clamp_bottom(camera_zoom, 1);
			}
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

		tm_scope("Move player")
		{
			// Update player position based on input
			Vector2 velocity = v2_scale(input_axis, player -> speed);

			player -> velocity = velocity;

			Vector2 movement = v2_scale(velocity, delta_t);

			update_entity(player, movement);
		}

		// Shader / Draw

		// Create bloom map and game image when window size changes (or first time)
		local_persist Os_Window last_window;
		if ((last_window.width != window.width || last_window.height != window.height || !game_image) && window.width > 0 && window.height > 0) 
		{
			if (bloom_map)   delete_image(bloom_map);
			if (game_image)  delete_image(game_image);
			if (final_image) delete_image(final_image);
			if (ui_image)    delete_image(ui_image);
			
			bloom_map  = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
			game_image = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
			final_image = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
			ui_image = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
		}
		last_window = window;
		
		// Set stuff in cbuffer which we need to pass to shaders
		scene_cbuffer.mouse_pos_screen = v2(input_frame.mouse_x, input_frame.mouse_y);
		scene_cbuffer.window_size = v2(window.width, window.height);
		
		// Draw game with light shader to game_image
		// Reset draw frame & clear the image with a clear color
		draw_frame_reset(& offscreen_draw_frame);
		gfx_clear_render_target(game_image, v4(0, 0, 0, 0));
		// Draw game things to offscreen Draw_Frame
		draw_game(current_draw_frame);
		
		// Set the shader & cbuffer before the render call
		//offscreen_draw_frame.shader_extension = light_shader;
		offscreen_draw_frame.cbuffer = & scene_cbuffer;
		
		// Render Draw_Frame to the image
		///// NOTE: Drawing to one frame like this will wait for the gpu to finish the last draw call. If this becomes
		// a performance bottleneck, you would have more frames "in flight" which you cycle through.
		gfx_render_draw_frame(& offscreen_draw_frame, game_image);
		
		// Draw game with bloom map shader to the bloom map
		
		// Reset draw frame & clear the image
		draw_frame_reset(& offscreen_draw_frame);
		gfx_clear_render_target(bloom_map, COLOR_BLACK);
		
		// Draw game things to offscreen Draw_Frame
		draw_game(current_draw_frame);
		
		// Set the shader & cbuffer before the render call
		offscreen_draw_frame.shader_extension = bloom_map_shader;
		offscreen_draw_frame.cbuffer = & scene_cbuffer;
		
		// Render Draw_Frame to the image
		///// NOTE: Drawing to one frame like this will wait for the gpu to finish the last draw call. If this becomes
		// a performance bottleneck, you would have more frames "in flight" which you cycle through.
		gfx_render_draw_frame(& offscreen_draw_frame, bloom_map);
		
		// Draw game image into final image, using the bloom shader which samples from the bloom_map
		
		draw_frame_reset(& offscreen_draw_frame);
		gfx_clear_render_target(final_image, COLOR_BLACK);
		
		// To sample from another image in the shader, we must bind it to a specific slot.
		draw_frame_bind_image_to_shader(& offscreen_draw_frame, bloom_map, 0);
		
		// Draw the game the final image, but now with the post process shader
		draw_image_in_frame(game_image, v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), COLOR_WHITE, & offscreen_draw_frame);
		
		offscreen_draw_frame.shader_extension = postprocess_bloom_shader;
		offscreen_draw_frame.cbuffer = & scene_cbuffer;
		
		gfx_render_draw_frame(& offscreen_draw_frame, final_image);

		// UI rendering

		draw_frame_reset(& ui_draw_frame);
		gfx_clear_render_target(ui_image, v4(0,0,0,0));

		ui_draw_frame.enable_z_sorting = true;
		ui_draw_frame.cbuffer = & scene_cbuffer;

		current_draw_frame = & ui_draw_frame;

		draw_ui(& ui_draw_frame);

		gfx_render_draw_frame(& ui_draw_frame, ui_image);
	
		switch (view) 
		{
			case VIEW_GAME_AFTER_POSTPROCESS:
			{
				draw_image(final_image, v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), COLOR_WHITE);
				Draw_Quad *q = draw_image(ui_image, v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), COLOR_WHITE);
				// The draw image will be flipped on y, so we want to draw it "upside down"
				swap(q -> uv.y, q -> uv.w, float);
				break;
			}

			case VIEW_GAME_BEFORE_POSTPROCESS:
			{
				draw_image(game_image, v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), COLOR_WHITE);
				break;
			}

			case VIEW_BLOOM_MAP:
			{
				draw_image(bloom_map, v2(-window.width / 2, -window.height / 2), v2(window.width, window.height), COLOR_WHITE);
				break;
			}

			default: break;
			{

			}
		}

		for (int i = 0; i < VIEW_MODE_MAX; i += 1) 
		{
			if (button(view_mode_stringify(i), v2(-window.width / 2 + 40, window.height / 2 - 100 -i * 60), v2(500, 50), i == view)) 
			{
				view = i;
			}
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
				memset(& world_frame, 0, sizeof(WorldFrame));
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