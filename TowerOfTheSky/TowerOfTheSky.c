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

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

Vector4 bg_box_color = {0, 0, 0, 0.5};

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

void collide_visual_debug(Entity *current_entity)
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
			draw_rect(v2(actor -> pos.x, actor -> pos.y), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2));  // Draw bounding box
			draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(sprite_data_2.image -> width, sprite_data_2.image -> height), v4(255, 0, 0, 0.2));  // Draw bounding box
			draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(1, 1), v4(0, 255, 255, 1)); // Where we are
		}
	}
}

void collide_visual_debug_buildings(Entity *current_entity)
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
            draw_rect(v2(building -> pos.x - half_tile_width, building -> pos.y - half_tile_width), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2));  // Draw bounding box
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

void update_debug_circle(DebugCircleState *state) 
{
    if (state -> active) 
    {
        if (state -> time_remaining > 0.0f) 
        {
            Vector2 circle_size = v2(state -> radius * 2.0f, state -> radius * 2.0f);
            Vector4 circle_color = v4(255, 0, 0, 1);

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
    // Entity setup
    en -> enemy_entity.entity_ID = ENTITY_Enemy;
    en -> enemy_entity.sprite_ID = SPRITE_Slime;
    en -> enemy_entity.health = 50;
    en -> enemy_entity.max_health = 50;
    en -> enemy_entity.health_regen = 0.2f;
    en -> enemy_entity.speed = 20;

    // Enemy setup
    en -> enemy_logic.enemy_state = ENEMYSTATE_idle;
    en -> enemy_logic.roam_direction = (Vector2){0, 0}; 
	en -> enemy_logic.damage = 5;
	en -> enemy_logic.attack_cooldown = 1;   
	en -> enemy_logic.current_attack_cooldown = 0;  
    en -> enemy_logic.attack_range = 25;   
    en -> enemy_logic.aggro_range = 100;  
    en -> enemy_logic.roam_time = 0;
    en -> enemy_logic.idle_time = 0;
    en -> enemy_logic.flee_time = 0;
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

void setup_player(Entity* player_en) 
{
	player_en -> entity_ID = ENTITY_Player;
    player_en -> sprite_ID = SPRITE_Player;
	player_en -> is_valid = true;
	player_en -> health = 100;
	player_en -> max_health = 100;
	player_en -> health_regen = 2;
	player_en -> speed = 75;
	player_en -> pos = v2(0, 0);
	player_en -> pos = round_v2_to_tile(player_en -> pos);
	player_en -> pos.y -= tile_width * 0.5;
	player_en -> pos.x -= sprites[player_en -> sprite_ID].image -> width * 0.5;

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

FloorData create_empty_floor(bool first_floor, int floor_ID)
{
	FloorData floor;
	memset(& floor, 0, sizeof(FloorData));

	create_circle_floor_data(& floor);

	setup_walls(& floor, floor_ID);

    setup_stairs(& floor, first_floor, floor_ID);

	if (first_floor == false)
	{
		setup_crates(& floor, 10, floor_ID);
	}

	floor.is_valid = true;
	floor.floor_ID = floor_ID;

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
			world -> floors[next_floor_id] = create_empty_floor(false, next_floor_id);
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
			world -> floors[next_floor_id] = create_empty_floor(false, next_floor_id);
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

void update_enemy_states(Enemy *enemy)
{
    if (enemy -> enemy_entity.is_valid == false) return;

	Entity *player = get_player();

	Vector2 player_center = v2((player -> pos.x + (sprites[player -> sprite_ID].image -> width * 0.5f)), (player -> pos.y + (sprites[player -> sprite_ID].image -> height * 0.5f)));

	float distance_to_player = v2_distance(enemy -> enemy_entity.pos, player_center);
	
	// wander
	if (distance_to_player >= enemy -> enemy_logic.aggro_range)
	{
		enemy -> enemy_logic.enemy_state = ENEMYSTATE_patrol;

		if (enemy -> enemy_logic.roam_time <= 0)
		{
			enemy -> enemy_logic.roam_time = 3;

			// random direction for wandering
			float random_angle = (float)(rand() % 360) * (PI32/ 180.0f);
			enemy -> enemy_logic.roam_direction.x = cosf(random_angle);
			enemy -> enemy_logic.roam_direction.y = sinf(random_angle);
		}
		else
		{
			update_cooldown(& enemy -> enemy_logic.roam_time);
		}
	
		Vector2 direction = enemy -> enemy_logic.roam_direction;

		Vector2 velocity = v2_scale(direction, enemy -> enemy_entity.speed);

		Vector2 movement = v2_scale(velocity, delta_t);

		update_entity(& enemy -> enemy_entity, movement);

		return;
	}

	// flee if low hp
	if (distance_to_player <= enemy -> enemy_logic.aggro_range && enemy -> enemy_entity.health < (enemy -> enemy_entity.max_health / 4))
	{
		enemy -> enemy_logic.enemy_state = ENEMYSTATE_flee;

		Vector2 direction = v2_add(player_center, enemy -> enemy_entity.pos);
		float length = v2_length(direction);

		if (length != 0.0f)
		{
			direction = v2_scale(direction, 1.0f / length);
		}

		Vector2 velocity = v2_scale(direction, enemy -> enemy_entity.speed);

		Vector2 movement = v2_scale(velocity, delta_t);

		update_entity(& enemy -> enemy_entity, movement);

		return;
	}

	// Only move enemy if close to player
	if (distance_to_player <= enemy -> enemy_logic.aggro_range)
	{
		enemy -> enemy_logic.enemy_state = ENEMYSTATE_combat;

		Vector2 direction = v2_sub(player_center, enemy-> enemy_entity.pos);
		float length = v2_length(direction);

		if (length != 0.0f)
		{
			direction = v2_scale(direction, 1.0f / length);
		}

		Vector2 velocity = v2_scale(direction, enemy -> enemy_entity.speed);

		Vector2 movement = v2_scale(velocity, delta_t);

		if (distance_to_player <= enemy -> enemy_logic.attack_range)
		{
			// Combat: Attack

			if (enemy -> enemy_logic.current_attack_cooldown <= 0)
			{
				enemy -> enemy_logic.current_attack_cooldown = enemy -> enemy_logic.attack_cooldown;

				damage_player(enemy -> enemy_logic.damage);
			}
			else
			{
				update_cooldown(& enemy -> enemy_logic.current_attack_cooldown);
			}
		}
			
		update_entity(& enemy -> enemy_entity, movement);

		return;
	}
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

                    Vector2 mouse_pos = get_mouse_pos_in_world_space();

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
	draw_frame.camera_xform = m4_scalar(1.0);
	draw_frame.projection = m4_make_orthographic_projection(0.0, screen_width, 0.0, screen_height, -1, 10);
}

void set_world_space() 
{
	draw_frame.projection = world_frame.world_proj;
	draw_frame.camera_xform = world_frame.world_view;
}

// :UI

void draw_resource_bar(Resource *resource, float y_pos, int icon_size, int icon_row_count, Vector4 color, Vector4 bg_color)
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

		current_resource_string = sprint(get_temporary_allocator(), current_resource_string, resource -> name, current_resource_int, max_resource_int, resource -> per_second);

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
		draw_rect_xform(xform, v2(bar_width, icon_size), bg_color);
	}

	// Bar Fill
	{
		Matrix4 xform = m4_identity;
		xform = m4_translate(xform, v3((position.x - (bar_width * 0.5)), (position.y + icon_size), 0.0));
		draw_rect_xform(xform, v2(bar_visual_size, icon_size), color);
	}	
}

Draw_Quad* draw_button(string button_tooltip, float button_size, Vector2 button_position, Vector4 color)
{
	Vector2 button_size_v2 = v2(16.0, 16.0);

	Matrix4 xform = m4_scalar(1.0);

	xform = m4_translate(xform, v3(button_position.x, button_position.y, 0.0));

	Vector2 icon_positon = v2(button_position.x, button_position.y);
	
	// White transparent box to show item slot is filled.
	Draw_Quad* quad = draw_rect_xform(xform, v2(button_size, button_size), color);

	// Setup box for mouse collision
	Range2f button_range = range2f_make_bottom_left(button_position, button_size_v2);

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
	Range2f button_range = range2f_make_bottom_left(button_pos, button_size);

	// Check if mouse is on button
	if (range2f_contains(button_range, get_mouse_pos_in_world_space())) 
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
	if (range2f_contains(button_range, get_mouse_pos_in_world_space())) 
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

void display_skill_level_up_button(float button_size, Vector4 color)
{
	Vector2 button_size_v2 = v2(button_size, button_size);
	int y_pos = 220;
	int current_buttons = 0;

	for (int i = 0; i < SKILLID_MAX; i++)
	{
		if (get_player_skill(skills[i].skill_ID) != NULL && get_player_skill(skills[i].skill_ID) -> unlocked == true)
		{
			current_buttons++;
			Vector2 button_pos = v2(100, (y_pos - (current_buttons * 30)));
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

			string button_text = sprint(get_temporary_allocator(), STR("%s\nLevel:%i\nCost: %s"), skill -> name, skill -> level, costs_text);
			string button_tooltip = sprint(get_temporary_allocator(), STR("%s\nLevel:%i\nCost: %s\n+%.2f %s \n%s"), skill -> name, skill -> level, costs_text, skill -> current_effect_value, skill -> effect_text, skill -> description);

			if (check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
			{
				world_frame.hover_consumed = true;
				find_skill_to_level(skills[i].skill_ID);
			}

			Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color);

			if (check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
			{
				world_frame.hover_consumed = true;
				quad -> color = COLOR_RED;
				draw_tooltip_box_string_to_side_larger(quad, button_size, & button_tooltip);
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

void display_upgrade_buttons(float button_size, Vector4 color)
{
	Vector2 button_size_v2 = v2(button_size, button_size);

	update_known_upgrades();

	int y_pos = 240;
	
	int current_buttons = 0;

	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (is_upgrade_in_known(upgrades[i].upgrade_ID) == true)
		{
			if (get_player_upgrade(upgrades[i].upgrade_ID) == NULL)
			{
				current_buttons++;

				Vector2 button_pos = v2(400, (y_pos - (current_buttons * 30)));

				string button_text = sprint(get_temporary_allocator(), STR(upgrades[i].name));

				string button_tooltip =  sprint(get_temporary_allocator(), STR(upgrades[i].description));

				Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color);	

				if (check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;

					unlock_upgrade(upgrades[i].upgrade_ID);
				}

				if (check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
				{
					world_frame.hover_consumed = true;
					quad -> color = COLOR_RED;
					draw_tooltip_box_string_to_side_larger(quad, button_size, & button_tooltip);
				}					
			}
		}
	}
}

void display_ability_upgrade_buttons(float button_size, Vector4 color)
{
	Vector2 button_size_v2 = v2(button_size, button_size);

	int y_pos = 20;
	
	int current_buttons = 0;

	for (int i = 0; i < UPGRADEID_MAX; i++)
	{
		if (get_player_upgrade(upgrades[i].upgrade_ID) != UPGRADEID_nil && get_player_upgrade(upgrades[i].upgrade_ID) -> has_levels == true)
		{
			current_buttons++;

			Vector2 button_pos = v2(100 + (current_buttons * 40), y_pos);

			string button_text = sprint(get_temporary_allocator(), STR("Level Up: %s \n Current Level: %i"), get_player_upgrade(upgrades[i].upgrade_ID) -> level_up_text, get_player_upgrade(upgrades[i].upgrade_ID) -> level);

			string button_tooltip = sprint(get_temporary_allocator(), STR(upgrades[i].description));

			Draw_Quad* quad = draw_button(button_text, button_size, button_pos, color);	

			if (check_if_mouse_clicked_button(button_pos, button_size_v2) == true)
			{
				world_frame.hover_consumed = true;
				level_up_upgrade(upgrades[i].upgrade_ID);
			}

			if (check_if_mouse_hovering_button(button_pos, button_size_v2) == true)
			{
				world_frame.hover_consumed = true;
				quad -> color = COLOR_RED;
				draw_tooltip_box_string_to_side_larger(quad, button_size, & button_tooltip);
			}					
		}
	}
}

void render_ui()
{
	set_screen_space();

	push_z_layer(Layer_UI);

	Vector2 txt_scale = v2(0.1, 0.1);
	Vector4 bg_col = v4(0, 0, 0, 0.90);
	Vector4 fill_col = v4(0.5, 0.5, 0.5, 1.0);
	Vector4 accent_col_blue = hex_to_rgba(0x44c3daff);
	Vector4 accent_col_purple = hex_to_rgba(0x9d00ffff);

	// :Fps Display

	string current_fps = tprint("%i FPS %.2f ms", fps_display, frame_time_display);

	draw_text_with_pivot(font, current_fps, font_height, v2(5, 265), v2(0.15, 0.15), COLOR_WHITE, PIVOT_top_left);

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

			const float icon_size = 16.0;

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
				draw_resource_bar(get_player_resource(RESOURCEID_Mana), 240, icon_size, icon_row_count, accent_col_blue, bg_box_color);
			}

			// Intellect bar
			if (get_player_resource(RESOURCEID_Intellect) != NULL && get_player_resource(RESOURCEID_Intellect) -> unlocked == true)
			{
				draw_resource_bar(get_player_resource(RESOURCEID_Intellect), 220, icon_size, icon_row_count, accent_col_purple, bg_box_color);
			}

			display_skill_level_up_button(16, fill_col);
			display_upgrade_buttons(16, fill_col);
			display_ability_upgrade_buttons(16, fill_col);
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

// :Floor Rendering

void render_floor_tiles(FloorData* floor, Vector4 color_0)
{	
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
			if (((x + y) % 2) == 0) 
			{
				col.a = 0.75;
			}
			
			float x_pos = x * tile_width;
			float y_pos = y * tile_width;
			
			draw_rect(v2(x_pos - half_tile_width, y_pos - half_tile_width), v2(tile_width, tile_width), col);
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

void render_buildings(FloorData* floor, Vector4 color_0)
{
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

			draw_image_xform(sprites[tile_data -> building.sprite_data.sprite_ID].image, xform, sprite_size, COLOR_WHITE);

			// pos debug
			//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), tile_data -> building.pos.x, tile_data -> building.pos.y), font_height, tile_data -> building.pos, v2(0.2, 0.2), COLOR_WHITE);
			
			// floor debug
			//draw_text(font, sprint(get_temporary_allocator(), STR("Current Floor:%i"), tile_data -> building.current_floor), font_height, tile_data -> building.pos, v2(0.2, 0.2), COLOR_WHITE);
		}
	}
}

// :Player Rendering

void render_player()
{
	Entity *player = get_player();
	SpriteData sprite_data = sprites[player -> sprite_ID];

	Vector2 sprite_size = get_sprite_size(sprite_data);

	Matrix4 xform = m4_scalar(1.0);

	xform = m4_translate(xform, v3(player -> pos.x, player -> pos.y, 0));

	Vector4 col = COLOR_WHITE;
	draw_image_xform(sprite_data.image, xform, sprite_size, col);

	// Healthbar test values
	Vector2 health_bar_pos = v2((player -> pos.x + (sprite_data.image -> width * 0.5)), (player -> pos.y + (sprite_data.image -> height)));

	// Temperary render player healthbar test
	draw_unit_bar(health_bar_pos, & player -> health, & player -> max_health, & player -> health_regen, 4, 6, COLOR_RED, bg_box_color);

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

void render_entities()
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

					draw_image_xform(sprites[en -> sprite_ID].image, xform, sprite_size, col);

					Vector2 health_bar_pos = v2((en -> pos.x + (sprites[en -> sprite_ID].image -> width * 0.5)), (en -> pos.y + (sprites[en -> sprite_ID].image -> height)));

					// Temp healthbar for non-players
					draw_unit_bar(health_bar_pos, & en -> health, & en -> max_health, & en -> health_regen, 4, 6, COLOR_RED, bg_box_color);

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

// :World setup

void world_setup()
{
	//start inventory open
	world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);

	if (world -> current_floor == null)
	{
		world -> current_floor = 0;
	}
	log("%i current floor", world -> current_floor);
	world -> floors[world -> current_floor] = create_empty_floor(true, world -> current_floor);
	world -> active_floors++;

	world -> player = hero_default;
	setup_player(& world -> player.player);

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

bool world_save_to_disk() 
{
	u64 everything_but_floors = offsetof(World, floors);

	u64 active_floors = world -> active_floors * sizeof(FloorData);

	u64 all_save_data = everything_but_floors + active_floors;

	return os_write_entire_file_s(STR("world"), (string){all_save_data, (u8*)world});
}

bool world_attempt_load_from_disk() 
{
    string result = {0};
    bool succ = os_read_entire_file_s(STR("world"), & result, get_heap_allocator()); // <--------------- Allocate on heap instead (temp is killed every frame)
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

    dealloc_string(get_heap_allocator(), result); // <-------------------- Dealloc after copied over

    return true;
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

	Vector4 color_0 = hex_to_rgba(0x2a2d3aff);

	load_sprite_data();
	load_resource_data();
	load_skill_data();
	load_ability_data();
	load_upgrade_data();

	setup_fireball_anim(); // Setup fireball animation so it can be used.

	// :Font Setup

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());

	assert(font, "Failed loading arial.ttf, %d", GetLastError());

	// Camera Settings
	float zoom = 1;
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

	string current_fps = STR("0");

	while (!window.should_close) 
	{
		reset_temporary_storage();
		world_frame = (WorldFrame){0};

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

			world_frame.world_view = m4_make_scale(v3(1.0, 1.0, 1.0));
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0.0)));
			world_frame.world_view = m4_mul(world_frame.world_view, m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0)));
		}

		set_world_space();

		push_z_layer(Layer_WORLD);

		Vector2 mouse_pos_world = get_mouse_pos_in_world_space();
		int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
		int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);

		// :World update
		{
			world -> time_elapsed += delta_t;
		}

		update_cooldown(& world -> floor_cooldown);

		FloorData* floor_data = & world -> floors[world -> current_floor];

		tm_scope("Render Floor Tiles")
		{
			render_floor_tiles(floor_data, color_0);
		}

		tm_scope("Render Buildings")
		{
			render_buildings(floor_data, color_0);
		}

		tm_scope("Render entities")
		{
			render_entities();
		}

		tm_scope("Render Player")
		{
			render_player();
		}

		tm_scope("Render UI")
		{
			render_ui();
		}

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
						if (get_player_resource(RESOURCEID_Mana) -> current >= get_player_ability(ABILITYID_Fire_Bolt) -> base_resource_cost)
						{
							spawn_projectile(get_player_ability(ABILITYID_Fire_Bolt), get_player(), 250.0, & Fireball, 1.0, 22, 1000, 5, false);

							get_player_resource(RESOURCEID_Mana) -> current -= get_player_ability(ABILITYID_Fire_Bolt) -> base_resource_cost;
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
						if (get_player_resource(RESOURCEID_Mana) -> current >= get_player_ability(ABILITYID_Fire_Bolt) -> base_resource_cost)
						{
							spawn_projectile(get_player_ability(ABILITYID_Fire_Bolt), get_player(), 250.0, & Fireball, 1.0, 22, 1000, 5, true);

							get_player_resource(RESOURCEID_Mana) -> current -= get_player_ability(ABILITYID_Fire_Bolt) -> base_resource_cost;
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

			Vector2 movement = v2_scale(velocity, delta_t);

			update_entity(player, movement);
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