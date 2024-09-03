#include "TowerOfTheSky.h"

// :Global APP

float64 delta_t;

Gfx_Font* font;

u32 font_height = 48;

float screen_width = 480.0;

float screen_height = 270.0;

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

// :Variables

#define m4_identity m4_make_scale(v3(1, 1, 1))

#define MAX_PROJECTILES 100 

Vector4 bg_box_color = {0, 0, 0, 0.5};

const float entity_selection_radius = 8.0f;

const float player_pickup_radius = 10.0f;

const int exp_vein_health = 3;

const int player_health = 100;

const int player_max_health = 100;

const int player_regen = 4;

const int target_health = 100;

const int target_max_health = 100;

const int target_regen = 25;

// :Testing Toggle

#define DEV_TESTING

// :Sprites

typedef struct SpriteData SpriteData;

struct SpriteData
{
	Gfx_Image* image;
};

typedef enum SpriteID SpriteID;

enum SpriteID
{
	SPRITE_nil,
	SPRITE_player,
	SPRITE_research_station,
	SPRITE_exp,
	SPRITE_exp_vein,
	SPRITE_fireball_sheet,
	SPRITE_target,
	SPRITE_MAX,
};

SpriteData sprites[SPRITE_MAX];

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

Vector2 get_sprite_size(SpriteData* sprite)
{
	return (Vector2) {sprite -> image -> width, sprite -> image -> height};
}

typedef enum ArchetypeID ArchetypeID;

enum ArchetypeID
{
	ARCH_nil = 0,
	ARCH_player = 1,
	ARCH_enemy = 2,
	ARCH_item = 3,
	ARCH_exp_vein = 4,
	ARCH_research_station = 5,
	ARCH_MAX,
};

// :Items

#define MAX_RECIPE_INGREDIENTS 8

typedef enum ItemID ItemID;

enum ItemID
{
	ITEM_nil,
	ITEM_rock,
	ITEM_pine_wood,
	ITEM_exp,
	ITEM_MAX,
};

typedef struct InventoryItemData InventoryItemData;

struct InventoryItemData 
{
	int amount;
};

typedef struct ItemAmount ItemAmount;

struct ItemAmount 
{
	ItemID id;
	int amount;
};

typedef struct ItemData ItemData;

struct ItemData
{
	string pretty_name;
	int amount;
	ArchetypeID for_structure;
	ItemAmount crafting_recipe[MAX_RECIPE_INGREDIENTS];
	float craft_length;
};

ItemData items[ITEM_MAX];

ItemData get_item_data(ItemID id)
{
	if (id >= 0 && id < ITEM_MAX)
	{
		return items[id];
	}
	return items[0];
}

SpriteID get_sprite_id_from_ItemID(ItemID item_id)
{
	switch (item_id)
	{

		case ITEM_exp: 
		{
			return SPRITE_exp;
			break;
		}

		default:
		{
			return 0;
		}
	}
}

string get_ItemID_pretty_name(ItemID item_id) 
{
	switch (item_id) 
	{
		case ITEM_exp: 
		{
			return STR("Exp");
			break;
		}

		default:
		{
			return STR("nil");
		}
	}
}

// :Entities

#define MAX_ENTITY_COUNT 1024

typedef struct Entity Entity;

struct Entity
{
	bool is_valid;
	bool render_sprite;
	SpriteID sprite_id;
	ArchetypeID arch;
	Vector2 pos;
	float health;
	float max_health;
	float health_regen;
	ItemID item;
	bool is_item;
	bool destroyable_world_item;
	bool is_immortal;
	float xRemainder;
	float yRemainder;
};

string get_archetype_pretty_name(ArchetypeID arch) 
{
	switch (arch) 
	{
		case ARCH_research_station:
		{
		return STR("Research Station");
		break;
		} 

		default:
		{
			return STR("nil");
		}
	}
}

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

    float speed;              
    float rotation;
    float damage;
    float scale;
    float radius;
	float xRemainder;
	float yRemainder;

	Entity *source_entity; // Caster of the Projectile
};

Projectile projectiles[MAX_PROJECTILES];

// :UX

typedef enum UXState UXState;

enum UXState
{
	UX_nil,
	UX_inventory,
	UX_building,
	UX_build_mode,
	UX_workbench,
	UX_research,
};

// :World

typedef struct World World;

struct World
{
	Entity entities[MAX_ENTITY_COUNT];

	InventoryItemData inventory_items[ITEM_MAX];

	UXState ux_state;

	float inventory_alpha;

	float inventory_alpha_target;

	float building_alpha;

	float building_alpha_target;
};

World* world = 0;

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	Entity* selected_entity;
	Matrix4 world_proj;
	Matrix4 world_view;
	bool hover_consumed;
	Entity* player;
};

WorldFrame world_frame;

// :Game

typedef struct Game Game;

struct Game 
{
Projectile *projectiles;
};

// :Setup

Entity* get_player() 
{
	return world_frame.player;
}

Entity* entity_create() 
{
	Entity* entity_found = 0;

	for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
	{
		Entity* existing_entity = & world -> entities[i];

		if (!existing_entity -> is_valid) 
		{
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "No more free entities!");
	entity_found -> is_valid = true;
	return entity_found;
}

void entity_destroy(Entity* entity) 
{
	memset(entity, 0, sizeof(Entity));
}

void setup_player(Entity* en) 
{
	en -> arch = ARCH_player;
	en -> sprite_id = SPRITE_player;
	en -> health = player_health;
	en -> max_health = player_max_health;
	en -> health_regen = player_regen;
}

void setup_target(Entity* en) 
{
	en -> arch = ARCH_enemy;
	en -> sprite_id = SPRITE_target;
	en -> health = target_health;
	en -> max_health = target_max_health;
	en -> health_regen = target_regen;
	en -> is_immortal = true;
}

void setup_item(Entity* en, ItemID item_id) 
{
	en -> arch = ARCH_item;
	en -> sprite_id = get_sprite_id_from_ItemID(item_id);
	en -> is_item = true;
	en -> item = item_id;
}

void entity_setup(Entity* en, ArchetypeID id) 
{
	switch (id) 
	{
		default: 
		{
			log_error("missing entity_setup case entry"); 
			break;
		}
	}
}

// :Functions

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
	ArchetypeID archetype = entity -> arch;

	//log("Debug: archetype = %i", archetype);  // Log the archetype value

	switch (archetype) 
	{
		case ARCH_player:
		{
			DamagePlayer(entity, damage);
			break;
		}

		case ARCH_enemy:
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
    SpriteData* sprite1 = get_sprite(current_entity -> sprite_id);
    int width1 = sprite1 -> image -> width;
    int height1 = sprite1 -> image -> height;

    int x_end1 = x + width1;
    int y_end1 = y + height1;

    for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
    {
        Entity *actor = &world -> entities[i];

        // Skip ourselves to avoid self-collision
        if (actor -> is_valid && actor != current_entity) 
        {
            SpriteData* sprite2 = get_sprite(actor -> sprite_id);
            int width2 = sprite2 -> image -> width;
            int height2 = sprite2 -> image -> height;

            int actor_x_end = actor -> pos.x + width2;
            int actor_y_end = actor-> pos.y + height2;

            // Check for bounding box overlap
            if (x < actor_x_end && x_end1 > actor -> pos.x &&
                y < actor_y_end && y_end1 > actor -> pos.y) 
            {
                log("Collision detected with entity %d\n", i);
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
		Entity *actor = & world -> entities[i];

		if (actor -> is_valid && actor != current_entity) 
		{
			SpriteData* sprite = get_sprite(actor -> sprite_id);
			int sprite_width = sprite -> image -> width;
			int sprite_height = sprite -> image -> height;

			SpriteData* sprite2 = get_sprite(current_entity -> sprite_id);

			// Visual Debug tools
			draw_rect(v2(actor -> pos.x, actor -> pos.y), v2(sprite_width, sprite_height), v4(255, 0, 0, 0.2));  // Draw bounding box
			draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(sprite2 -> image -> width, sprite2 -> image -> height), v4(255, 0, 0, 0.2));  // Draw bounding box
			draw_rect(v2(current_entity -> pos.x, current_entity -> pos.y), v2(1, 1), v4(0, 255, 255, 1)); // Where we are
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

// Distance between two vectors
float32 v2_distance(Vector2 a, Vector2 b) 
{
    return v2_length(v2_sub(a, b));
}

// Vector scaling
Vector2 v2_scale(Vector2 v, float32 scale) 
{
    return (Vector2){v.x * scale, v.y * scale};
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

void spawn_projectile(Entity *source_entity, float speed, float damage, AnimationInfo *animation, float32 scale, float spawn_radius) 
{
    for (int i = 0; i < MAX_PROJECTILES; i++) 
    {
        if (!projectiles[i].is_active) 
        {
            Projectile *projectile = & projectiles[i];
            projectile -> is_active = true;
            projectile -> speed = speed;
            projectile -> damage = damage;
            projectile -> animation = *animation;
            projectile -> scale = scale;
            projectile -> source_entity = source_entity;

            // Calculate the player's center position
            SpriteData *sprite = get_sprite(source_entity -> sprite_id);
            Vector2 player_center = v2((source_entity -> pos.x + (sprite -> image -> width * 0.5f)), (source_entity -> pos.y + (sprite->image->height * 0.5f)));

            Vector2 mouse_pos = get_mouse_pos_in_world_space();

            // Direction from the player to mouse
            Vector2 direction = v2_sub(mouse_pos, player_center);
            float32 length = v2_length(direction);

            // Normalize
            if (length != 0.0f)
            {
                direction = v2_scale(direction, 1.0f / length);
            }

            // Calculate the angle to spawn the projectile
            float angle = atan2f(direction.y, direction.x);
            Vector2 spawn_position = v2_add(player_center, v2(spawn_radius * cosf(angle), spawn_radius * sinf(angle)));

            projectile->position = spawn_position;

            // Debug logging
            // printf("Player Center: (%f, %f)\n", player_center.x, player_center.y);
            // printf("Spawn Position: (%f, %f)\n", spawn_position.x, spawn_position.y);
            // printf("Mouse Position: (%f, %f)\n", mouse_pos.x, mouse_pos.y);
            // printf("Projectile Direction Angle: %f\n", angle);
            // printf("Spawn Radius: %f\n", spawn_radius);

            // Draw the debug circle around the player when projectile is cast
            start_debug_circle(& circle_state, player_center, spawn_radius, 1.0);

            // Calculate projectile velocity and rotation
            Vector2 velocity_direction = v2_sub(mouse_pos, spawn_position);
            float32 velocity_length = v2_length(velocity_direction);

            if (velocity_length != 0.0f)
            {
                velocity_direction = v2_scale(velocity_direction, 1.0f / velocity_length);
            }

            projectile -> velocity = v2_scale(velocity_direction, speed);
            projectile -> rotation = atan2f(-velocity_direction.y, velocity_direction.x) * (180.0f / PI32);

            if (projectile -> rotation < 0.0f)
            {
                projectile -> rotation += 360.0f;
            }
            break;
        }
    }
}

Entity* projectile_collides_with_entity(Projectile *projectile) 
{
    for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
    {
        Entity *entity = & world -> entities[i];

        if (entity -> is_valid && entity != projectile -> source_entity) // Skip the source entity
        {
            SpriteData* sprite = get_sprite(entity -> sprite_id);
            int entity_width = sprite -> image -> width;
            int entity_height = sprite -> image -> height;

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

    // Check if the projectile hits any entity
    Entity *hit_entity = projectile_collides_with_entity(projectile);

    if (hit_entity) 
    {
        DamageEntity(hit_entity, projectile -> damage);

        // Deactivate the projectile after it hits an entity
        projectile -> is_active = false;
        return;
    }

    // Check if the projectile has reached the target position
    if (v2_distance(projectile -> position, projectile -> target_position) < projectile -> speed * delta_time) 
    {
        projectile -> position = projectile -> target_position; // Ensure it reaches the target
        projectile -> is_active = false;  // Deactivate the projectile
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
	{
		Gfx_Text_Metrics metrics = measure_text(font, *title, font_height, v2(0.1, 0.1));

		Vector2 draw_pos = icon_center;

		draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
		
		draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

		draw_pos = v2_add(draw_pos, v2(0, tooltip_size * -0.5));

		draw_pos = v2_add(draw_pos, v2(0, -2.0)); // Padding

		draw_text(font, *title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

		current_y_pos = draw_pos.y;
	}
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

// pad_pct just shrinks the rect by a % of itself ... 0.2 is a nice default

Draw_Quad* draw_sprite_in_rect(SpriteID sprite_id, Range2f rect, Vector4 col, float pad_pct) 
{
	SpriteData* sprite = get_sprite(sprite_id);
	Vector2 sprite_size = get_sprite_size(sprite);

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

	} else if (sprite_size.y > sprite_size.x) 
	{ // tall boi
		
		// width is a ratio of height
		Vector2 range_size = range2f_size(rect);
		rect.max.x = rect.min.x + (range_size.y * (sprite_size.x/sprite_size.y));
		// center along the X
		float new_width = rect.max.x - rect.min.x;
		rect = range2f_shift(rect, v2((range_size.x - new_width) * 0.5, 0));
	}

	return draw_image(sprite -> image, rect.min, range2f_size(rect), col);
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
				InventoryItemData* item = & world -> inventory_items[i];

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
				InventoryItemData* item = & world -> inventory_items[id];

				if (item -> amount > 0)
				{
					// Draw item icons
					float slot_index_offset = slot_index * icon_size;

					Matrix4 xform = m4_scalar(1.0);

					xform = m4_translate(xform, v3(x_start_pos + slot_index_offset, y_pos, 0.0));
					SpriteData* sprite = get_sprite(get_sprite_id_from_ItemID(id));
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

					draw_sprite_in_rect(get_sprite_id_from_ItemID(id), box, COLOR_WHITE, 0.2); // New sprite rendering from randy day 11
				
					// Tooltip
					if (is_selected_alpha == 1.0)
					{
						string name = get_ItemID_pretty_name(id);

						string title = sprint(get_temporary_allocator(), STR("%s\nx%i"), name, item -> amount);

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

	//start inventory open
	world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);

	// :Load Sprites

	// Missing Texture Sprite
	sprites[0] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/missing_tex.png"), get_heap_allocator())};
	
	// Player
	sprites[SPRITE_player] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/player.png"), get_heap_allocator())};

	// Items
	sprites[SPRITE_exp] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp.png"), get_heap_allocator())};
	sprites[SPRITE_exp_vein] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/exp_vein.png"), get_heap_allocator())};

	// Buildings
	sprites[SPRITE_research_station] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/research_station.png"), get_heap_allocator())};
	sprites[SPRITE_target] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/Target.png"), get_heap_allocator())};

	// Spells
	sprites[SPRITE_fireball_sheet] = (SpriteData){ .image = load_image_from_disk(STR("TowerOfTheSky/Resources/Sprites/fireball_sprite_sheet.png"), get_heap_allocator())};
	
	setup_fireball_anim(); // Setup fireball animation so it can be used.

	#if CONFIGURATION == DEBUG
		{
			for (SpriteID i = 0; i < SPRITE_MAX; i++) 
			{
				SpriteData* sprite = & sprites[i];
				assert(sprite -> image, "Sprite was not setup properly");
			}
		}
	#endif

	// :Font Setup

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());

	assert(font, "Failed loading arial.ttf, %d", GetLastError());

	// Entity Setup
	{
		Entity* player_en = entity_create();
		setup_player(player_en);

		// :Test stuff
		#if defined(DEV_TESTING)
		{
			world -> inventory_items[ITEM_exp].amount = 50;

			// Spawn one Target
			Entity* en = entity_create();
			setup_target(en);
			en -> pos = v2(0, 40);
			en -> pos = round_v2_to_tile(en -> pos);
		}
		#endif
	}

	// Camera Settings
	float zoom = 4;
	Vector2 camera_pos = v2(0, 0);

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
			Entity* en = & world -> entities[i];
			if (en -> is_valid && en -> arch == ARCH_player) 
			{
				world_frame.player = en;
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

		// Debug Visuals
		update_debug_circle(& circle_state);

		// :Do UI Rendering

		do_ui_stuff();

		// :Tile Rendering
		{
			int player_tile_x = world_pos_to_tile_pos(get_player() -> pos.x);
			int player_tile_y = world_pos_to_tile_pos(get_player() -> pos.y);
			int tile_radius_x = 40; 
			int tile_radius_y = 30;

			for (int x = player_tile_x - tile_radius_x; x < player_tile_x + tile_radius_x; x++) 
			{
				for (int y = player_tile_y - tile_radius_y; y < player_tile_y + tile_radius_y; y++) 
				{
					Vector4 color_0 = hex_to_rgba(0x2a2d3aff);
					Vector4 col = color_0;

					if ((x + (y % 2 == 0) ) % 2 == 0) 
					{
						col.a = 0.8;
					}
					float x_pos = x * tile_width;
					float y_pos = y * tile_width;
					draw_rect(v2(x_pos + tile_width * -0.5, y_pos + tile_width * -0.5), v2(tile_width, tile_width), col);
				}
			}
			// Show which tile is currently selected
			//draw_rect(v2(tile_pos_to_world_pos(mouse_tile_x) + tile_width * -0.5, tile_pos_to_world_pos(mouse_tile_y) + tile_width * -0.5), v2(tile_width, tile_width), /*v4(0.5, 0.5, 0.5, 0.5)*/ v4(0.5, 0.0, 0.0, 1.0));
		}

		// :Render Entities

		for (int i = 0; i < MAX_ENTITY_COUNT; i++)
		{
			Entity* en = & world -> entities[i];

			if (en -> is_valid)
			{
				switch (en -> arch)
				{	
					case ARCH_player:
					{
						break;
					}

					default:
					{
						SpriteData* sprite = get_sprite(en -> sprite_id);

						// Get sprite dimensions
						Vector2 sprite_size = get_sprite_size(sprite);

						Matrix4 xform = m4_scalar(1.0);

						if(en -> is_item == true)
						{
							xform = m4_translate(xform, v3(0, 2 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0));
						}

						xform = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));

						Vector4 col = COLOR_WHITE;

						if(world_frame.selected_entity == en)
						{
							col = COLOR_RED;
						}

						draw_image_xform(sprite -> image, xform, sprite_size, col);

						Vector2 health_bar_pos = v2((en -> pos.x + + (sprite -> image -> width * 0.5)), (en -> pos.y + (sprite -> image -> height)));

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
			if (projectiles[i].is_active) 
			{
				update_projectile(& projectiles[i], delta_t);

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
			SpriteData *sprite = get_sprite(player -> sprite_id);

			Vector2 sprite_size = get_sprite_size(sprite);

			Matrix4 xform = m4_scalar(1.0);

			xform = m4_translate(xform, v3(player -> pos.x, player -> pos.y, 0));

			Vector4 col = COLOR_WHITE;
			draw_image_xform(sprite->image, xform, sprite_size, col);

			// Healthbar test values
			Vector2 health_bar_pos = v2((player -> pos.x + (sprite -> image -> width * 0.5)), (player -> pos.y + (sprite -> image -> height)));

			// Temperary render player healthbar test
			draw_unit_bar(health_bar_pos, & player -> health, & player -> max_health, & player -> health_regen, 4, 6, COLOR_RED, bg_box_color);

			// World space current location debug for object pos
			draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), player -> pos.x, player -> pos.y), font_height, player -> pos, v2(0.2, 0.2), COLOR_WHITE);

			if(is_key_just_pressed(KEY_F3))
			{
				// Draw the debug circle around the player
            	// start_debug_circle(& circle_state, player -> pos, 80, 0.5);

				// Ghetto Fireball Spawn Test
				float fireball_cost = (mana.current - 5);

				if(mana.current >= fireball_cost)
				{
					spawn_projectile(player, 300.0, 10.0, & Fireball, 1.0, 30.0);
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

		// Update player position
		updateEntity(player, v2_mulf(input_axis, 100.0 * delta_t));

		os_update(); 
		gfx_update();
	}

	return 0;
}