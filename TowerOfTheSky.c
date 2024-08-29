#include "Range.c"

inline float v2_dist(Vector2 a, Vector2 b) 
{
    return v2_length(v2_sub(a, b));
}

typedef enum Pivot Pivot;

enum Pivot 
{
	PIVOT_bottom_left,
	PIVOT_bottom_center,
	PIVOT_bottom_right,
	PIVOT_center_left,
	PIVOT_center_center,
	PIVOT_center_right,
	PIVOT_top_left,
	PIVOT_top_center,
	PIVOT_top_right,
};

void draw_text_with_pivot(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Pivot pivot) 
{
	Gfx_Text_Metrics metrics = measure_text(font, text, raster_height, scale);
	position = v2_sub(position, metrics.visual_pos_min);
	Vector2 pivot_mul = {0};

	switch (pivot) 
	{
		case PIVOT_bottom_left:
		{
			pivot_mul = v2(0.0, 0.0);
			break;
		} 

		case PIVOT_center_center:
		{
			pivot_mul = v2(0.5, 0.5); 
			break;
		} 

		case PIVOT_center_left:
		{
			pivot_mul = v2(0.0, 0.5);
			break;
		} 

		case PIVOT_top_center:
		{
			pivot_mul = v2(0.5, 1.0);
			break;
		} 

		default:
		{
			log_error("pivot not supported yet. fill in case at draw_text_with_pivot");
			break;
		}
	}

	position = v2_sub(position, v2_mul(metrics.visual_size, pivot_mul));
	draw_text(font, text, raster_height, position, scale, color);
}

bool almost_equals(float a, float b, float epsilon) 
{
 	return fabs(a - b) <= epsilon;
}

bool animate_f32_to_target(float* value, float target, float delta_t, float rate) 
{
	*value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
	if (almost_equals(*value, target, 0.001f))
	{
		*value = target;
		return true; // Reached
	}
	return false;
}

void animate_v2_to_target(Vector2* value, Vector2 target, float delta_t, float rate) 
{
	animate_f32_to_target(&(value -> x), target.x, delta_t, rate);
	animate_f32_to_target(&(value -> y), target.y, delta_t, rate);
}

Range2f quad_to_range(Draw_Quad quad) 
{
	return (Range2f){quad.bottom_left, quad.top_right};
}

float float_alpha(float x, float min, float max) 
{
	float res = (x-min) / (max-min);
	res = clamp(res, 0.0, 1.0);
	return res;
}

inline float64 now() 
{
	return os_get_elapsed_seconds();
}

float alpha_from_end_time(float64 end_time, float length) 
{
	return float_alpha(now(), end_time-length, end_time);
}

bool has_reached_end_time(float64 end_time) 
{
	return now() > end_time;
}

Draw_Quad ndc_quad_to_screen_quad(Draw_Quad ndc_quad) 
{
	// NOTE: we're assuming these are the screen space matricies.
	Matrix4 proj = draw_frame.projection;
	Matrix4 view = draw_frame.camera_xform;

	Matrix4 ndc_to_screen_space = m4_identity();
	ndc_to_screen_space = m4_mul(ndc_to_screen_space, m4_inverse(proj));
	ndc_to_screen_space = m4_mul(ndc_to_screen_space, view);

	ndc_quad.bottom_left = m4_transform(ndc_to_screen_space, v4(v2_expand(ndc_quad.bottom_left), 0, 1)).xy;
	ndc_quad.bottom_right = m4_transform(ndc_to_screen_space, v4(v2_expand(ndc_quad.bottom_right), 0, 1)).xy;
	ndc_quad.top_left = m4_transform(ndc_to_screen_space, v4(v2_expand(ndc_quad.top_left), 0, 1)).xy;
	ndc_quad.top_right = m4_transform(ndc_to_screen_space, v4(v2_expand(ndc_quad.top_right), 0, 1)).xy;

	return ndc_quad;
}

Vector2 get_mouse_pos_in_ndc()
{
	float mouse_x = input_frame.mouse_x;
	float mouse_y = input_frame.mouse_y;
	Matrix4 proj = draw_frame.projection;
	Matrix4 view = draw_frame.camera_xform;
	float window_w = window.width;
	float window_h = window.height;

	// Normalize the mouse coordinates
	float ndc_x = (mouse_x / (window_w * 0.5f)) - 1.0f;
	float ndc_y = (mouse_y / (window_h * 0.5f)) - 1.0f;

	return (Vector2){ndc_x, ndc_y};
}

Vector2 get_mouse_pos_in_world_space() 
{
	float mouse_x = input_frame.mouse_x;
	float mouse_y = input_frame.mouse_y;
	Matrix4 proj = draw_frame.projection;
	Matrix4 view = draw_frame.camera_xform;
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

// :Utilities

float sin_breathe(float time, float rate)
{
	return (sin(time * rate) + 1.0 / 2.0);
}

// :Tile Functions

const int tile_width = 16;

int world_pos_to_tile_pos(float world_pos) 
{
	return roundf(world_pos / (float)tile_width);
}

float tile_pos_to_world_pos(int tile_pos) 
{
	return ((float)tile_pos * (float)tile_width);
}

Vector2 round_v2_to_tile(Vector2 world_pos) 
{
	world_pos.x = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.x));
	world_pos.y = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.y));
	return world_pos;
}

// :Global APP

float64 delta_t;

Gfx_Font* font;

u32 font_height = 48;

float screen_width = 480.0;

float screen_height = 270.0;

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

// :Wizard Testing stuffs

const float base_cost_multiplier = 1.1;

const float base_power_multiplier = 1.05;

float cost_multiplier = 1.005;

float power_multiplier = 1.0005;

float no_second_cost = 0.0;

float no_second_resource = 0.0;

// Mana

bool mana_unlocked = true;

float current_mana = 0.0;

float max_mana = 100.0;

float mana_per_second = 10.0;

// Intellect

bool intellect_unlocked = false;

float current_intellect = 0.0;

float max_intellect = 50.0;

float intellect_per_second = 0.0;

// Channel Mana

bool channel_mana_known = true;

int channel_mana_level = 0;

const float channel_mana_base_cost = 25.0;

float channel_mana_current_cost = channel_mana_base_cost;

const float channel_mana_base_mana_per_second_buff = 2.0;

float channel_mana_current_mana_per_second_buff = channel_mana_base_mana_per_second_buff;

const float channel_mana_base_cost_multiplier = base_cost_multiplier;

float channel_mana_current_cost_multiplier = channel_mana_base_cost_multiplier;

const float channel_mana_base_power_multiplier = base_power_multiplier;

float channel_mana_current_power_multiplier = channel_mana_base_power_multiplier;

// Wisdom

bool wisdom_known = false;

int wisdom_level = 0;

const float wisdom_base_cost = 1;

float wisdom_current_cost = wisdom_base_cost;

const float wisdom_base_max_mana_buff = 100;

float wisdom_current_max_mana_buff = wisdom_base_max_mana_buff;

const float wisdom_base_cost_multiplier = base_cost_multiplier;

float wisdom_current_cost_multiplier = wisdom_base_cost_multiplier;

const float wisdom_base_power_multiplier = base_power_multiplier;

float wisdom_current_power_multiplier = wisdom_base_power_multiplier;

// Focus

bool focus_known = false;

int focus_level = 0;

const float focus_base_cost = 50;

const float focus_base_cost_2 = 1;

float focus_current_cost = focus_base_cost;

float focus_current_cost_2 = focus_base_cost_2;

const float focus_base_intellect_per_second_buff = 0.20;

float focus_current_intellect_per_second_buff = focus_base_intellect_per_second_buff;

const float focus_base_cost_multiplier = base_cost_multiplier;

float focus_current_cost_multiplier = focus_base_cost_multiplier;

const float focus_base_power_multiplier = base_power_multiplier;

float focus_current_power_multiplier = focus_base_power_multiplier;

typedef struct LevelUpParams LevelUpParams;

struct LevelUpParams
{
	float *statToBuff; // Which Stat to Increase
	float *buffAmount; // Increase by how much
	float *statBuffMultiplier; // Increase the buff multiplier
    float *currentResourceSpent; // Resource type spent
	float *currentResourceSpent_2; // Resource type spent 2
    float *currentCost; // Current Cost for resource 1
    float *currentCost2;  // Current Cost for resource 2
	float *costMultiplier; // Increase the cost multiplier
    int *level;
};

void LevelUp(LevelUpParams *params)
{
	// Apply upgrade (buff the relevant attribute)
    *(params -> statToBuff) += *(params -> buffAmount);

    // Power up the upgrade
    *(params -> buffAmount) *= *(params -> statBuffMultiplier);

	// Increase power multiplier
	*(params -> statBuffMultiplier) *= power_multiplier;

    // Spend resources on the upgrade
    *(params -> currentResourceSpent) -= *(params -> currentCost);

	// Spend 2nd resource (if needed)
	if (*(params -> currentCost2) > 0)
	{
		*(params -> currentResourceSpent_2) -= *(params -> currentCost2);
	}

    // Increase the cost of the upgrade
    *(params -> currentCost) *= *(params -> costMultiplier);

	// Increase cost multiplier
	*(params -> costMultiplier) *= cost_multiplier;

    // Level up the upgrade
    (*(params -> level))++;
}

void LevelUpChannelMana()
{
    LevelUpParams params = 
	{
		& mana_per_second,
		& channel_mana_current_mana_per_second_buff,
		& channel_mana_current_power_multiplier,
		& current_mana, // resource spent 1
		& no_second_resource,
        & channel_mana_current_cost,
        & no_second_cost,
		& channel_mana_current_cost_multiplier,
        & channel_mana_level,
    };

    LevelUp(& params);
}

void LevelUpWisdom()
{
    LevelUpParams params = 
	{
		& max_mana,
		& wisdom_current_max_mana_buff,
		& wisdom_current_power_multiplier,
        & current_intellect, // resource spent 1
		& no_second_resource, 
        & wisdom_current_cost,
        & no_second_cost,
		& wisdom_current_cost_multiplier,
        & wisdom_level,
    };

    LevelUp(& params);
}

void LevelUpFocus()
{
    LevelUpParams params = 
	{
		& intellect_per_second,
		& focus_current_intellect_per_second_buff,
		& focus_current_power_multiplier,
        & current_mana, // resource spent 1
		& current_intellect, //resource spent 2
        & focus_current_cost,
        & focus_current_cost_2,
		& focus_current_cost_multiplier,
        & focus_level
    };

    LevelUp(& params);
}

void draw_level_up_button(string button_tooltip, Vector2 button_size, Vector2 button_position, Vector4 color)
{
	// Setup box for mouse collision
	Range2f btn_range = range2f_make_bottom_left(button_position, button_size);

	// Draw Button
	draw_rect(button_position, button_size, color);

	// Draw Button Text
	Gfx_Text_Metrics metrics = measure_text(font, button_tooltip, font_height, v2(0.1, 0.1));
	Vector2 draw_pos = v2((button_position.x + (button_size.x * 0.5)), (button_position.y + (button_size.y * 0.5)));
	draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
	draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

	draw_text(font, button_tooltip, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
}
							
// :Variables

#define m4_identity m4_make_scale(v3(1, 1, 1))

Vector4 bg_box_color = {0, 0, 0, 0.5};

const float entity_selection_radius = 8.0f;

const float player_pickup_radius = 10.0f;

const int exp_vein_health = 3;

const int player_health = 10;

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
	ARCH_item = 2,
	ARCH_exp_vein = 3,
	ARCH_research_station = 4,
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
	//ItemID type;
	int amount;
	// :recipe crafting
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

int get_crafting_recipe_count(ItemData item_data) 
{
	int count = 0;
	for (int i = 0; i < MAX_RECIPE_INGREDIENTS; i++) 
	{
		if (item_data.crafting_recipe[i].id == 0) 
		{
			break;
		}
		count += 1;
	}
	return count;
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
	int health;
	ItemID item;
	ItemID selected_crafting_item;
	bool destroyable_world_item;
	bool is_item;
	bool workbench_thing;
	ItemID current_crafting_item;
	int current_crafting_amount;
	float64 crafting_end_time;
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

// :Buildings

typedef struct BuildingData BuildingData;

struct BuildingData
{
	ArchetypeID to_build;
	SpriteID icon;
	int pct_per_research_exp; // this jank will get replaced with a recipe one day
	// Display Name
	// Cost
};

typedef enum BuildingID BuildingID;

enum BuildingID
{
	BUILDING_nil,
	BUILDING_workbench,
	BUILDING_research_station,
	BUILDING_MAX,
};

BuildingData buildings[BUILDING_MAX];

BuildingData get_building_data(BuildingID id)
{
    if (id >= 0 && id < BUILDING_MAX)
    {
        return buildings[id];
    }
    return buildings[0];
}

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
typedef struct UnlockState UnlockState;

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

	BuildingID placing_building;

	Entity* interacting_with_entity;

	ItemID selected_crafting_item;
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
}

void setup_exp_vein(Entity* en) 
{
	en -> arch = ARCH_exp_vein;
	en -> sprite_id = SPRITE_exp_vein;
	en -> health = exp_vein_health;
	en -> destroyable_world_item = true;
}

void setup_research_station(Entity* en) 
{
	en -> arch = ARCH_research_station;
	en -> sprite_id = SPRITE_research_station;
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
		case ARCH_research_station:
		{
			setup_research_station(en);
			break;
		}

		default: 
		{
			log_error("missing entity_setup case entry"); 
			break;
		}
	}
}

// :Functions

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
	if (sprite_size.x > sprite_size.y) { // long boi

		// height is a ratio of width
		Vector2 range_size = range2f_size(rect);
		rect.max.y = rect.min.y + (range_size.x * (sprite_size.y/sprite_size.x));
		// center along the Y
		float new_height = rect.max.y - rect.min.y;
		rect = range2f_shift(rect, v2(0, (range_size.y - new_height) * 0.5));

	} else if (sprite_size.y > sprite_size.x) { // tall boi
		
		// width is a ratio of height
		Vector2 range_size = range2f_size(rect);
		rect.max.x = rect.min.x + (range_size.y * (sprite_size.x/sprite_size.y));
		// center along the X
		float new_width = rect.max.x - rect.min.x;
		rect = range2f_shift(rect, v2((range_size.x - new_width) * 0.5, 0));
	}

	return draw_image(sprite->image, rect.min, range2f_size(rect), col);
}

Draw_Quad* draw_sprite_in_rect_test(SpriteID sprite_id, Matrix4 xform, Range2f rect, Vector4 col, float pad_pct) 
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
	{ 
		// long boi

		// height is a ratio of width
		Vector2 range_size = range2f_size(rect);
		rect.max.y = rect.min.y + (range_size.x * (sprite_size.y / sprite_size.x));
		// center along the Y
		float new_height = rect.max.y - rect.min.y;
		rect = range2f_shift(rect, v2(0, (range_size.y - new_height) * 0.5));

	} else if (sprite_size.y > sprite_size.x) 
	{ 
		// tall boi
		
		// width is a ratio of height
		Vector2 range_size = range2f_size(rect);
		rect.max.x = rect.min.x + (range_size.y * (sprite_size.x / sprite_size.y));
		// center along the X
		float new_width = rect.max.x - rect.min.x;
		rect = range2f_shift(rect, v2((range_size.x - new_width) * 0.5, 0));
	}

	return draw_image_xform(sprite -> image, xform, range2f_size(rect), col);
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

			float entire_thing_width = icon_row_count * icon_size;
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
					draw_sprite_in_rect_test(get_sprite_id_from_ItemID(id), xform, box, COLOR_WHITE, 0.0); // test?

					//draw_image_xform(sprite -> image, xform, v2(icon_size, icon_size), COLOR_WHITE); // old

					//draw_sprite_in_rect(get_sprite_id_from_ItemID(id), box, COLOR_WHITE, 0.2); // New sprite rendering from randy day 11
				
					// Tooltip
					if (is_selected_alpha == 1.0)
					{
						Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

						Range2f screen_range = quad_to_range(screen_quad);

						Vector2 icon_center = range2f_get_center(screen_range);

						Matrix4 xform = m4_scalar(1.0);

						Vector2 box_size = v2(16.0, 16.0);

						//xform = m4_pivot_box(xform, box_size, PIVOT_top_center);

						xform = m4_translate(xform, v3(box_size.x * -0.5, - box_size.y - icon_size * 0.5, 0));

						xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

						draw_rect_xform(xform, box_size, bg_box_color);

						float current_y_pos = icon_center.y;
						
						// Draw item name on screen
						{
							string title = get_ItemID_pretty_name(id);

							Gfx_Text_Metrics metrics = measure_text(font, title, font_height, v2(0.1, 0.1));

							Vector2 draw_pos = icon_center;

							draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
							
							draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

							draw_pos = v2_add(draw_pos, v2(0, icon_size * -0.5));

							draw_pos = v2_add(draw_pos, v2(0, -2.0)); // Padding

							draw_text(font, title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

							current_y_pos = draw_pos.y;
						}

						// Draw item amount on screen
						{
							string item_amount = STR("x%i"); // %i is where the number goes.

							item_amount = sprint(get_temporary_allocator(), item_amount , item -> amount);

							Gfx_Text_Metrics metrics = measure_text(font, item_amount, font_height, v2(0.1, 0.1));

							Vector2 draw_pos = v2(icon_center.x, current_y_pos);

							draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
							
							draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

							draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

							draw_text(font, item_amount, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
						}
					}
					slot_index += 1;
				}
			}

			// Mana bar
			if(mana_unlocked == true)
			{
				{
					float y_pos = 240;

					float mana_bar_width = icon_size * icon_row_count;

					float x_start_pos = (screen_width * 0.025);

					int current_mana_int = (int)current_mana;

					int max_mana_int = (int)max_mana;

					float percentage_of_manabar = (mana_bar_width / 100.0);

					float current_mana_percentage = (current_mana / max_mana) * 100.0f;

					float mana_bar_visual_size = (percentage_of_manabar * current_mana_percentage);

					if(current_mana < max_mana)
					{
						current_mana += mana_per_second * delta_t;
					}

					// Mana Overflow Check
					if(current_mana >= max_mana)
					{
						current_mana = max_mana;
					}

					if(channel_mana_level >= 5)
					{
						wisdom_known = true;

						//unlock intellect for now for testing other things (shouldn't be unlocked yet)

						if (intellect_unlocked == false)
						{
							intellect_unlocked = true;
							intellect_per_second = 0.5;
						}
						
					}

					// Black background box
					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
						draw_rect_xform(xform, v2(mana_bar_width, icon_size), bg_box_color);
					}

					// Mana fill
					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
						draw_rect_xform(xform, v2(mana_bar_visual_size, icon_size), accent_col_blue);
					}	

					// Mana bar current mana display
					{
						string current_mana_string = STR("Mana: %i/%i    +%.1f/s"); // %i is where the number goes.

						current_mana_string = sprint(get_temporary_allocator(), current_mana_string, current_mana_int, max_mana_int, mana_per_second);

						Gfx_Text_Metrics metrics = measure_text(font, current_mana_string, font_height, v2(0.20, 0.20));

						Vector2 draw_pos = v2(x_start_pos + (mana_bar_width * 0.5), y_pos + 14);

						draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
						
						draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

						draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

						draw_text(font, current_mana_string, font_height, draw_pos, v2(0.20, 0.20), COLOR_WHITE);
					}
				}
			}

			// intellect bar
			if(intellect_unlocked == true)
			{
				{
					float y_pos = 220;

					float intellect_bar_width = icon_size * icon_row_count;

					float x_start_pos = (screen_width * 0.025);

					int current_intellect_int = (int)current_intellect;

					int max_intellect_int = (int)max_intellect;

					float percentage_of_intellect = (intellect_bar_width / 100.0);

					float current_intellect_percentage = (current_intellect / max_intellect) * 100.0f;

					float intellect_bar_visual_size = (percentage_of_intellect * current_intellect_percentage);

					if(current_intellect < max_intellect)
					{
						current_intellect += intellect_per_second * delta_t;
					}

					// intellect Overflow Check
					if(current_intellect >= max_intellect)
					{
						current_intellect = max_intellect;
					}

					if(wisdom_level >= 5)
					{
						focus_known = true;
					}

					// Black background box
					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
						draw_rect_xform(xform, v2(intellect_bar_width, icon_size), bg_box_color);
					}

					//  intellect fill
					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
						draw_rect_xform(xform, v2(intellect_bar_visual_size, icon_size), accent_col_purple);
					}

					// intellect bar current intellect display
					{
						string current_intellect_string = STR("Intellect: %i/%i    +%.1f/s"); // %i is where the number goes.

						current_intellect_string = sprint(get_temporary_allocator(), current_intellect_string, current_intellect_int, max_intellect_int, intellect_per_second);

						Gfx_Text_Metrics metrics = measure_text(font, current_intellect_string, font_height, v2(0.20, 0.20));

						Vector2 draw_pos = v2(x_start_pos + (intellect_bar_width * 0.5), y_pos + 14);

						draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
						
						draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.25))); // Top center

						draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

						draw_text(font, current_intellect_string, font_height, draw_pos, v2(0.20, 0.20), COLOR_WHITE);
					}
				}
			}

			// Setup for all buttons for now
			Vector2 button_size = v2(16.0 , 16.0);

			// Level Up Channel Mana Button
			if(channel_mana_known == true)
			{
				{
					Vector2 button_pos = v2(150, y_pos);

					Vector4 color = fill_col;

					if(check_if_mouse_hovering_button(button_pos, button_size) == true)
					{
						world_frame.hover_consumed = true;
						color = COLOR_RED;
					}

					// Check if enough mana for upgrade
					if(current_mana > channel_mana_current_cost) 
					{
						if(check_if_mouse_clicked_button(button_pos, button_size) == true)
						{
							world_frame.hover_consumed = true;

							LevelUpChannelMana();
						}
					}

					string channel_mana_tooltip = sprint(get_temporary_allocator(), STR("Channel Mana\nLevel:%i\nCost: %.1f Mana\n+%.2f Base Mana / second"), channel_mana_level, channel_mana_current_cost, channel_mana_current_mana_per_second_buff);
				
					draw_level_up_button(channel_mana_tooltip, button_size, button_pos, color);	
				}
			}

			// Level Up wisdom Button
			if(wisdom_known == true)
			{
				{
					Vector2 button_pos = v2(150, y_pos - 30);

					Vector4 color = fill_col;

					if(check_if_mouse_hovering_button(button_pos, button_size) == true)
					{
						world_frame.hover_consumed = true;
						color = COLOR_RED;
					}
					
					// Check if enough mana for upgrade
					if(current_intellect > wisdom_current_cost)
					{
						if(check_if_mouse_clicked_button(button_pos, button_size) == true)
						{
							world_frame.hover_consumed = true;

							LevelUpWisdom();
						}
					}

					string wisdom_tooltip = sprint(get_temporary_allocator(), STR("Wisdom\nLevel:%i\nCost: %.1f Intellect\n+%.1f Max Mana"), wisdom_level, wisdom_current_cost, wisdom_current_max_mana_buff);

					draw_level_up_button(wisdom_tooltip, button_size, button_pos, color);	
				}
			}
			
			// Level Up Focus Button
			if(focus_known == true)
			{
				{
					Vector2 button_pos = v2(150, y_pos - 60);

					Vector4 color = fill_col;

					if(check_if_mouse_hovering_button(button_pos, button_size) == true)
					{
						world_frame.hover_consumed = true;
						color = COLOR_RED;
					}
					
					// Check if enough mana & intellect for upgrade
					if(current_mana > focus_current_cost && current_intellect > focus_current_cost_2)
					{
						if(check_if_mouse_clicked_button(button_pos, button_size) == true)
						{
							world_frame.hover_consumed = true;

							LevelUpFocus();
						}
					}

					string focus_tooltip = sprint(get_temporary_allocator(), STR("Focus\nLevel:%i\nCost: %.1f Mana + %.1f Intellect\n+%.1f Base Intellect / second"), focus_level, focus_current_cost, focus_current_cost_2, focus_current_intellect_per_second_buff);

					draw_level_up_button(focus_tooltip, button_size, button_pos, color);	
				}
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

	window.scaled_width = 1920; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 1080; 
	window.fullscreen = true;

	// Where on the monitor the window starts up at
	window.x = 0;
	window.y = 0;

	window.clear_color = hex_to_rgba(0x2a2d3aff);

	world = alloc(get_heap_allocator(), sizeof(World));

	float64 last_time = os_get_elapsed_seconds();

	//start inventory open
	world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);

	// :Load Sprites

	// Missing Texture Sprite
	sprites[0] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/missing_tex.png"), get_heap_allocator())};
	
	// Player
	sprites[SPRITE_player] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/player.png"), get_heap_allocator())};

	// Items
	sprites[SPRITE_exp] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/exp.png"), get_heap_allocator())};
	sprites[SPRITE_exp_vein] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/exp_vein.png"), get_heap_allocator())};

	// Buildings
	sprites[SPRITE_research_station] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/research_station.png"), get_heap_allocator())};

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

		{
			// Setup
			Entity* player_en = entity_create();
			setup_player(player_en);

			// :Test stuff
			#if defined(DEV_TESTING)
			{
				world -> inventory_items[ITEM_exp].amount = 50;
			}
			#endif
		}

	// Camera Settings
	float zoom = 3;
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
			if (en -> is_valid && en->arch == ARCH_player) 
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
					if ((x + (y % 2 == 0) ) % 2 == 0) 
					{
						Vector4 col = v4(0.0, 0.0, 0.2, 0.35);
						float x_pos = x * tile_width;
						float y_pos = y * tile_width;
						draw_rect(v2(x_pos + tile_width * -0.5, y_pos + tile_width * -0.5), v2(tile_width, tile_width), col);
					}
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
						Matrix4 xform = m4_scalar(1.0);

						if(en -> is_item == true)
						{
							xform = m4_translate(xform, v3(0, 2 * sin_breathe(os_get_elapsed_seconds(), 5.0), 0));
						}

						// @Volatile with entity placement

						xform = m4_translate(xform, v3(0, tile_width * -0.5, 0));
						xform = m4_translate(xform, v3(en -> pos.x, en -> pos.y, 0));
						xform = m4_translate(xform, v3(sprite -> image -> width * -0.5, 0.0, 0));
						
						Vector4 col = COLOR_WHITE;

						if(world_frame.selected_entity == en)
						{
							col = COLOR_RED;
						}

						draw_image_xform(sprite -> image, xform, get_sprite_size(sprite), col);

						//world space current location debug for object pos
						//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), en -> pos.x, en -> pos.y), font_height, en -> pos, v2(0.1, 0.1), COLOR_WHITE);

						break;
					}
				}
			}
		}

		//Render player
		{
			Entity* en = get_player();
			SpriteData* sprite = get_sprite(en->sprite_id);
			Matrix4 xform = m4_scalar(1.0);
			xform         = m4_translate(xform, v3(0, tile_width * -0.5, 0));
			xform         = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
			xform         = m4_translate(xform, v3(get_sprite_size(sprite).x * -0.5, 0.0, 0));

			Vector4 col = COLOR_WHITE;
			draw_image_xform(sprite->image, xform, get_sprite_size(sprite), col);
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

		get_player() -> pos = v2_add(get_player() -> pos, v2_mulf(input_axis, 100.0 * delta_t));

		os_update(); 
		gfx_update();
	}

	return 0;
}