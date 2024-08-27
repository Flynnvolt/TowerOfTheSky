// Rangef needed to continue tutorial

typedef struct Range1f 
{
  float min;
  float max;
} Range1f;

typedef struct Range2f 
{
  Vector2 min;
  Vector2 max;
} Range2f;

inline Range2f range2f_make(Vector2 min, Vector2 max) 
{ return (Range2f) { min, max }; }

Range2f range2f_shift(Range2f r, Vector2 shift) 
{
  r.min = v2_add(r.min, shift);
  r.max = v2_add(r.max, shift);
  return r;
}

Range2f range2f_make_bottom_center(Vector2 size) 
{
  Range2f range = {0};
  range.max = size;
  range = range2f_shift(range, v2(size.x * -0.5, 0.0));
  return range;
}

Vector2 range2f_size(Range2f range) 
{
  Vector2 size = {0};
  size = v2_sub(range.min, range.max);
  size.x = fabsf(size.x);
  size.y = fabsf(size.y);
  return size;
}

bool range2f_contains(Range2f range, Vector2 v) 
{
  return v.x >= range.min.x && v.x <= range.max.x && v.y >= range.min.y && v.y <= range.max.y;
}

Range2f range2f_make(Vector2 min, Vector2 max);
Range2f range2f_shift(Range2f r, Vector2 shift);
Range2f range2f_make_bottom_center(Vector2 size);
Vector2 range2f_size(Range2f range);
bool range2f_contains(Range2f range, Vector2 v);

Vector2 range2f_get_center(Range2f r) 
{
	return (Vector2) { (r.max.x - r.min.x) * 0.5 + r.min.x, (r.max.y - r.min.y) * 0.5 + r.min.y };
}

Range2f range2f_make_bottom_right(Vector2 pos, Vector2 size) 
{
  return (Range2f) {pos, v2_add(pos, size)};
}

inline float v2_dist(Vector2 a, Vector2 b) 
{
    return v2_length(v2_sub(a, b));
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

float screen_width = 240.0;

float screen_height = 135.0;

const s32 Layer_UI = 20;

const s32 Layer_WORLD = 10;

// :Variables

#define m4_identity m4_make_scale(v3(1, 1, 1))

Vector4 bg_box_color = {0, 0, 0, 0.5};

const float entity_selection_radius = 8.0f;

const float player_pickup_radius = 20.0f;

const int player_health = 10;

const int rock_health = 3;

const int tree_health = 3;

const int furnace_health = 3;

const int workbench_health = 3;

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
	SPRITE_tree0,
	SPRITE_tree1,
	SPRITE_rock0,
	SPRITE_item_gold,
	SPRITE_item_pine_wood,
	SPRITE_rock1,
	SPRITE_building_furnace,
	SPRITE_building_workbench,
	SPRITE_research_station,
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

typedef enum EntityArchetype EntityArchetype;

enum EntityArchetype
{
	ARCH_nil = 0,
	ARCH_player = 1,
	ARCH_item = 2,
	ARCH_rock = 3,
	ARCH_tree = 4,
	ARCH_tree2 = 5,
	ARCH_furnace = 6,
	ARCH_workbench = 7,
	ARCH_research_station = 8,
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
	EntityArchetype for_structure;
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
		case ITEM_pine_wood: 
		{
			return SPRITE_item_pine_wood;
			break;
		}

		case ITEM_rock: 
		{
			return SPRITE_item_gold;
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
		case ITEM_pine_wood:
		{
			return STR("Pine Wood");
			break;

		} 

		case ITEM_rock: 
		{
			return STR("Gold");
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
	EntityArchetype arch;
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

string get_archetype_pretty_name(EntityArchetype arch) 
{
	switch (arch) 
	{
		case ARCH_furnace:
		{
		return STR("Furnace");
		break;
		} 

		case ARCH_workbench:
		{
		return STR("WorkBench");
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
	EntityArchetype to_build;
	SpriteID icon;
	// Display Name
	// Cost
};

typedef enum BuildingID BuildingID;

enum BuildingID
{
	BUILDING_nil,
	BUILDING_furnace,
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

	BuildingID placing_building;

	Entity* open_workbench;

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
};

WorldFrame world_frame;

// :Setup

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

void setup_furnace(Entity* en) 
{
	en -> arch = ARCH_furnace;
	en -> sprite_id = SPRITE_building_furnace;
	en -> health = furnace_health;
	en -> workbench_thing = true;
}

void setup_workbench(Entity* en) 
{
	en -> arch = ARCH_workbench;
	en -> sprite_id = SPRITE_building_workbench;
	en -> health = workbench_health;
	en -> workbench_thing = true;
}

void setup_research_station(Entity* en) 
{
	en -> arch = ARCH_research_station;
	en -> sprite_id = SPRITE_research_station;
}

void setup_rock(Entity* en) 
{
	en -> arch = ARCH_rock;
	en -> sprite_id = SPRITE_rock0;
	en -> health = rock_health;
	en -> destroyable_world_item = true;
}

void setup_tree(Entity* en) 
{
	en -> arch = ARCH_tree;
	en -> sprite_id = SPRITE_tree0;
	en -> health = tree_health;
	en -> destroyable_world_item = true;
}

void setup_item(Entity* en, ItemID item_id) 
{
	en -> arch = ARCH_item;
	en -> sprite_id = get_sprite_id_from_ItemID(item_id);
	en -> is_item = true;
	en -> item = item_id;
}

void entity_setup(Entity* en, EntityArchetype id) 
{
	switch (id) 
	{
		case ARCH_furnace:
		{
			setup_furnace(en); 
			break;
		}

		case ARCH_workbench:
		{
			setup_workbench(en); 
			break;
		}

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

// :Functions

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

void do_ui_stuff()
{
	set_screen_space();

	push_z_layer(Layer_UI);

	// :Inventory UI
	{
		if(is_key_just_pressed(KEY_TAB))
		{
			consume_key_just_pressed(KEY_TAB);
			world -> ux_state = (world -> ux_state == UX_inventory ? UX_nil : UX_inventory);
		}

		world -> inventory_alpha_target = (world -> ux_state == UX_inventory ? 1.0 : 0.0);
		animate_f32_to_target(& world -> inventory_alpha, world -> inventory_alpha_target, delta_t, 200.0); //speed inventory closes / fades
		bool is_inventory_enabled = world -> inventory_alpha_target == 1;

		if(world -> inventory_alpha != 0.0)
		{
			// TODO make inventory fade in out when key pressed.

			float y_pos = 115.0;

			int item_count = 0;

			for (int i = 0; i < ITEM_MAX; i++)
			{
				InventoryItemData* item = & world -> inventory_items[i];

				if (item -> amount > 0)
				{
					item_count += 1;
				}
			}

			const float icon_thing = 16.0;
			float icon_width = icon_thing;

			const int icon_row_count = 8;

			float entire_thing_width = icon_row_count * icon_width;
			float x_start_pos = (screen_width * 0.5) - (entire_thing_width * 0.5);

			// Black background box
			{
				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
				draw_rect_xform(xform, v2(entire_thing_width, icon_width), bg_box_color);
			}

			int slot_index = 0;
			for (int id = 1; id < ITEM_MAX; id++)
			{
				InventoryItemData* item = & world -> inventory_items[id];

				if (item -> amount > 0)
				{
					// Draw item icons
					float slot_index_offset = slot_index * icon_width;

					Matrix4 xform = m4_scalar(1.0);
					xform = m4_translate(xform, v3(x_start_pos + slot_index_offset, y_pos, 0.0));

					SpriteData* sprite = get_sprite(get_sprite_id_from_ItemID(id));

					Draw_Quad* quad = draw_rect_xform(xform, v2(16, 16), v4(1, 1, 1, 0.2));

					Range2f icon_box = quad_to_range(*quad);

					float is_selected_alpha = 0.0;
				
					if (is_inventory_enabled && range2f_contains(icon_box, get_mouse_pos_in_ndc()))
					{
						is_selected_alpha = 1.0;
					}
				
					Matrix4 box_bottom_right_xform = xform;

					xform = m4_translate(xform, v3(icon_width * 0.5, icon_width * 0.5, 0.0));

					// Make items bigger when selected
					if (is_selected_alpha == 1.0)
					{
						// TODO selection polish
						float scale_adjust = 0.3;
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

					xform = m4_translate(xform, v3(get_sprite_size(sprite).x * -0.5,  get_sprite_size(sprite).y * -0.5, 0));
				
					draw_image_xform(sprite -> image, xform, get_sprite_size(sprite), COLOR_WHITE);

					// Tooltip
					if (is_selected_alpha == 1.0)
					{
						Draw_Quad screen_quad = ndc_quad_to_screen_quad(*quad);

						Range2f screen_range = quad_to_range(screen_quad);

						Vector2 icon_center = range2f_get_center(screen_range);

						Matrix4 xform = m4_scalar(1.0);

						// TODO - we're guessing at the Y box size here.
						// in order to automate this, we would need to move it down below after we do all the element advance things for the text.
						// ... but then the box would be drawing in front of everyone. So we'd have to do Z sorting.
						// Solution for now is to just guess at the size.
						Vector2 box_size = v2(40.0, 15.0);

						//xform = m4_pivot_box(xform, box_size, PIVOT_top_center);

						xform = m4_translate(xform, v3(box_size.x * -0.5, - box_size.y - icon_width * 0.5, 0));

						xform = m4_translate(xform, v3(icon_center.x, icon_center.y, 0));

						draw_rect_xform(xform, box_size, bg_box_color);

						float current_y_pos = icon_center.y;
						
						// Draw item name on screen
						{
							string title = get_ItemID_pretty_name(id);

							Gfx_Text_Metrics metrics = measure_text(font, title, font_height, v2(0.1, 0.1));

							Vector2 draw_pos = icon_center;

							draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
							
							draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.0))); // Top center

							draw_pos = v2_add(draw_pos, v2(0, icon_width * -0.5));

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
							
							draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.0))); // Top center

							draw_pos = v2_add(draw_pos, v2(0, -2.0)); // padding

							draw_text(font, item_amount, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
						}
					}

					slot_index += 1;
				}
			}
		}
	}

	// :Building UI

	{
		if(is_key_just_pressed('C'))
		{
			consume_key_just_pressed('C');
			world -> ux_state = (world -> ux_state == UX_building ? UX_nil : UX_building);
		}

		world -> building_alpha_target = (world -> ux_state == UX_building ? 1.0 : 0.0);
		animate_f32_to_target(& world -> building_alpha, world -> building_alpha_target, delta_t, 200.0); // Speed building closes / fades
		bool is_building_enabled = world -> building_alpha_target == 1;

		if(world -> building_alpha != 0.0)
		{
			float y_pos = 10.0;

			float icon_size = 16.0;

			float padding = 4.0;

			float building_count = (BUILDING_MAX - 1); // One less because nil is not a building.

			float total_box_width = (building_count * icon_size) + (padding * (building_count + 1));

			float x_start_pos = (screen_width * 0.5) - (total_box_width * 0.5);

			// Black background box
			{
				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(x_start_pos, y_pos, 0.0));
				draw_rect_xform(xform, v2(total_box_width, icon_size), bg_box_color);
			}

			float x_start = x_start_pos + padding;
			
			// Draw building Icon
			for (BuildingID id = 1; id < BUILDING_MAX; id++)
			{
				BuildingData* building = & buildings[id];

				Matrix4 xform = m4_identity;
				xform = m4_translate(xform, v3(x_start, y_pos, 0.0));
				
				//draw_rect_xform(xform, v2(icon_size, icon_size), v4(1, 1, 1, 0.2)); // White Background box 

				SpriteData* sprite = get_sprite(building -> icon);

				// TODO: Fix sprites so they are not stretched
				Draw_Quad* quad = draw_image_xform(sprite -> image, xform, v2(icon_size, icon_size), COLOR_WHITE);

				Range2f box = quad_to_range(*quad);

				if (range2f_contains(box, get_mouse_pos_in_ndc()))
				{
					world_frame.hover_consumed = true;

					if(is_key_just_pressed(MOUSE_BUTTON_LEFT))
					{
						consume_key_just_pressed(MOUSE_BUTTON_LEFT);
						
						world -> placing_building = id;
						world -> ux_state = UX_build_mode;
					}
				}

				// Move to the position for the next icon
				x_start += icon_size + padding;
			}
		}
	}

	// :Building Mode

	if(world -> ux_state == UX_build_mode)
	{
		set_world_space();

		{
			Vector2 mouse_pos_world = get_mouse_pos_in_world_space();

			BuildingData building = get_building_data(world -> placing_building);
			SpriteData* sprite = get_sprite(building.icon);

			Vector2 pos = mouse_pos_world;
			pos = round_v2_to_tile(pos);

			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, v3(pos.x, pos.y, 0));

			// @Volatile with entity rendering

			xform = m4_translate(xform, v3(0, tile_width * -0.5, 0));
			xform = m4_translate(xform, v3(get_sprite_size(sprite).x * -0.5, 0.0, 0));

			draw_image_xform(sprite -> image, xform, get_sprite_size(sprite), COLOR_WHITE);

			if (is_key_just_pressed(MOUSE_BUTTON_LEFT))
			{
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);

				Entity* en = entity_create();

				entity_setup(en, building.to_build);

				en -> pos = pos;

				world -> ux_state = 0;
			}
		}

		set_screen_space();
	}

	// :Workbench UI

	if (world -> ux_state == UX_workbench && world -> open_workbench) 
	{
		Entity* workbench_en = world -> open_workbench;

		Vector2 section_size = v2(50.0, 70.0);
		float gap_between_panels = 10.0;
		float text_height_pad = 4.0;
		Vector4 bg_col = v4(0, 0, 0, 0.7);
		Vector4 fill_col = v4(0.5, 0.5, 0.5, 1.0);

		float ui_width_thing = section_size.x * 2.0 + gap_between_panels;

		float x0 = screen_width * 0.5 - ui_width_thing * 0.5;
		float y0 = screen_height * 0.5 - section_size.y * 0.5;
		{
			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, v3(x0, y0, 0));
			draw_rect_xform(xform, section_size, bg_col);

			// title
			float x1 = x0;
			float y1 = y0 + section_size.y;
			{
				string title = get_archetype_pretty_name(workbench_en -> arch);
				Gfx_Text_Metrics metrics = measure_text(font, title, font_height, v2(0.1, 0.1));

				float center_pos = x0 + section_size.x * 0.5;
				Vector2 draw_pos = v2(center_pos, y1);
				draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
				draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.0))); // top center
				draw_pos.y -= text_height_pad;

				draw_text(font, title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

				y1 = draw_pos.y; // TODO - workie?
				y1 -= text_height_pad;
				// y1 -= 20.0;
			}

			Vector2 item_icon_size = v2(8, 8);

			y1 -= item_icon_size.y;

			// draw all item recipes
			for (int i = 1; i < ITEM_MAX; i++) 
			{
				ItemData data = get_item_data(i);
				if (data.for_structure == workbench_en -> arch && get_crafting_recipe_count(data)) 
				{
					Matrix4 xform = m4_identity;
					xform = m4_translate(xform, v3(x1, y1, 0));

					Vector4 col = COLOR_WHITE;
					if (world -> selected_crafting_item == i) 
					{
						col = COLOR_RED;
					}

					Draw_Quad* quad = draw_image_xform(get_sprite(get_sprite_id_from_ItemID(i)) -> image, xform, item_icon_size, col);
					Range2f icon_box = quad_to_range(*quad);
					if (range2f_contains(icon_box, get_mouse_pos_in_ndc())) 
					{
						// ...
						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) 
						{
							consume_key_just_pressed(MOUSE_BUTTON_LEFT);
							world -> selected_crafting_item = i;
						}
					}
					x1 += item_icon_size.x;
				}
			}
		}

		x0 += section_size.x;
		x0 += gap_between_panels;
		Vector2 bottom_left_right_pane = v2(x0, y0);
		Vector2 top_left_right_pane = v2(x0, y0 + section_size.y);

		// right pane
		{
			Matrix4 xform = m4_identity;
			xform = m4_translate(xform, v3(x0, y0, 0));
			draw_rect_xform(xform, section_size, bg_col);

			if (world -> selected_crafting_item) 
			{
				ItemData selected_item_data = get_item_data(world -> selected_crafting_item);

				// title of item
				y0 += section_size.y;
				{
					string title = selected_item_data.pretty_name;
					Gfx_Text_Metrics metrics = measure_text(font, title, font_height, v2(0.1, 0.1));

					float center_pos = x0 + section_size.x * 0.5;
					Vector2 draw_pos = v2(center_pos, y0);
					draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
					draw_pos = v2_add(draw_pos, v2_mul(metrics.visual_size, v2(-0.5, -1.0))); // top center
					draw_pos.y -= text_height_pad;

					draw_text(font, title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);

					y0 = draw_pos.y; // TODO - workie?
					y0 -= text_height_pad;
					// y1 -= 20.0;
				}

				y0 = bottom_left_right_pane.y + 30.0; // @cleanup

				// list out the ingredients
				for (int i = 0; i < get_crafting_recipe_count(selected_item_data); i++) 
				{
					ItemAmount ingredient_amount = selected_item_data.crafting_recipe[i];
					ItemData ingredient_item_data = get_item_data(ingredient_amount.id);

					Vector2 element_size = v2(section_size.x * 0.8, 6.0);

					x0 = bottom_left_right_pane.x + (section_size.x - element_size.x) * 0.5;

					// bg box thing
					draw_rect(v2(x0, y0), element_size, fill_col);

					// icon
					{
						float item_icon_length = element_size.y * 0.8;

						float x1 = bottom_left_right_pane.x + section_size.x * 0.5 - element_size.y;
						float y1 = y0 + (item_icon_length - element_size.y) * -0.5;

						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(x1, y1, 0));
						Draw_Quad* quad = draw_image_xform(get_sprite(get_sprite_id_from_ItemID(ingredient_amount.id))->image, xform, v2(item_icon_length, item_icon_length), COLOR_WHITE);
						Range2f icon_box = quad_to_range(*quad);

						if (range2f_contains(icon_box, get_mouse_pos_in_ndc())) 
						{
							// ...
						}
					}

					InventoryItemData inv_item = world -> inventory_items[ingredient_amount.id];

					Vector4 txt_col = COLOR_WHITE;
					if (inv_item.amount < ingredient_amount.amount) 
					{
						txt_col = COLOR_RED;
					}

					string txt = tprint("%i/%i", inv_item.amount, ingredient_amount.amount);

					Gfx_Text_Metrics metrics = measure_text(font, txt, font_height, v2(0.1, 0.1));
					float center_pos = bottom_left_right_pane.x + section_size.x * 0.5;
					Vector2 draw_pos = v2(center_pos, y0 + element_size.y * 0.5);
					draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
					draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0, 0.5)));

					draw_text(font, txt, font_height, draw_pos, v2(0.1, 0.1), txt_col);

					// y0 += metrics.visual_size.y;

					y0 -= element_size.y;

					y0 -= 2.0f; // padding @cleanup
				}
				
				// craft button
				{
					Vector2 size = v2(section_size.x * 0.8, 6.0);

					x0 = bottom_left_right_pane.x + (section_size.x - size.x) * 0.5;
					y0 = bottom_left_right_pane.y;
					y0 += 5.0f; // padding from bottom @cleanup

					// check for all ingredients met
					bool has_enough_for_crafting = true;
					for (int i = 0; i < get_crafting_recipe_count(selected_item_data); i++) 
					{
						ItemAmount ingredient_amount = selected_item_data.crafting_recipe[i];
						if (ingredient_amount.amount > world -> inventory_items[ingredient_amount.id].amount) 
						{
							has_enough_for_crafting = false;
							break;
						}
					}

					Range2f btn_range = range2f_make_bottom_right(v2(x0, y0), size);
					Vector4 col = fill_col;
					if (has_enough_for_crafting && range2f_contains(btn_range, get_mouse_pos_in_world_space()))
					{
						col = COLOR_RED;
						world_frame.hover_consumed = true;
						// TODO - where do we put the state for the animation of the button?
						// Either just manually rip it via the App state, or some kind of hash string thing that's probably overcomplicated as shit.
						if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) 
						{
							consume_key_just_pressed(MOUSE_BUTTON_LEFT);
							
							// :Craft!

							workbench_en -> current_crafting_item = world -> selected_crafting_item;
							workbench_en -> current_crafting_amount += 1;

							// remove ingredients from inventory
							for (int i = 0; i < get_crafting_recipe_count(selected_item_data); i++) 
							{
								ItemAmount ingredient_amount = selected_item_data.crafting_recipe[i];
								world -> inventory_items[ingredient_amount.id].amount -= ingredient_amount.amount;
								assert(world -> inventory_items[ingredient_amount.id].amount >= 0, "removed too many items. the check above must have failed!");
							}
						}
					}
					// todo - disable button with has_enough_for_crafting
					draw_rect(v2(x0, y0), size, col);

					string txt = STR("CRAFT");
					Gfx_Text_Metrics metrics = measure_text(font, txt, font_height, v2(0.1, 0.1));
					float center_pos = bottom_left_right_pane.x + section_size.x * 0.5;
					Vector2 draw_pos = v2(center_pos, y0 + size.y * 0.5);
					draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
					draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

					draw_text(font, txt, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
				}
			} 
			
			else 
			{
				// select item first text
				y0 += section_size.y * 0.5;
				{
					string title = STR("Select Item to Craft");
					Gfx_Text_Metrics metrics = measure_text(font, title, font_height, v2(0.1, 0.1));

					Vector2 draw_pos = v2(x0 + section_size.x * 0.5, y0);
					draw_pos = v2_sub(draw_pos, metrics.visual_pos_min);
					draw_pos = v2_sub(draw_pos, v2_mul(metrics.visual_size, v2(0.5, 0.5)));

					draw_text(font, title, font_height, draw_pos, v2(0.1, 0.1), COLOR_WHITE);
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
	window.title = STR("Tutorial Test");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x2a2d3aff);

	world = alloc(get_heap_allocator(), sizeof(World));

	float64 last_time = os_get_elapsed_seconds();

	// :Load Sprites

	// Missing Texture Sprite
	sprites[0] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/missing_tex.png"), get_heap_allocator())};
	
	// Player
	sprites[SPRITE_player] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/player.png"), get_heap_allocator())};

	// Trees / Rocks
	sprites[SPRITE_tree0] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/tree0.png"), get_heap_allocator())};
	sprites[SPRITE_tree1] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/tree1.png"), get_heap_allocator())};
	sprites[SPRITE_rock0] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/rock0.png"), get_heap_allocator())};
	sprites[SPRITE_rock1] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/rock1.png"), get_heap_allocator())};

	// Items
 	sprites[SPRITE_item_gold] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/gold.png"), get_heap_allocator())};
	sprites[SPRITE_item_pine_wood] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/pine_wood.png"), get_heap_allocator())};

	// Buildings
	sprites[SPRITE_building_furnace] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/furnace.png"), get_heap_allocator())};
	sprites[SPRITE_building_workbench] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/workbench.png"), get_heap_allocator())};
	sprites[SPRITE_research_station] = (SpriteData){ .image = load_image_from_disk(STR("Resources/Sprites/research_station.png"), get_heap_allocator())};

	#if CONFIGURATION == DEBUG
		{
			for (SpriteID i = 0; i < SPRITE_MAX; i++) 
			{
				SpriteData* sprite = & sprites[i];
				assert(sprite->image, "Sprite was not setup properly");
			}
		}
		#endif

	// :Font Setup

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());

	render_atlas_if_not_yet_rendered(font, 32, 'A');

	// :Building Resource Setup	

	{
		//buildings[0] =
		buildings[BUILDING_furnace] = (BuildingData) {.to_build = ARCH_furnace, .icon = SPRITE_building_furnace};
		buildings[BUILDING_workbench] = (BuildingData) {.to_build = ARCH_workbench, .icon = SPRITE_building_workbench};
		buildings[BUILDING_research_station] = (BuildingData){ .to_build = ARCH_research_station, .icon = SPRITE_research_station };
	}

	// :item data resource setup

	{
		// do defaults
		for (ItemID i = 1; i < ITEM_MAX; i++) 
		{
			ItemData* data = & items[i];
			data -> craft_length = 2.0;
		}

		items[ITEM_rock] = (ItemData){ .pretty_name = STR("Rock"), .for_structure = ARCH_furnace, .crafting_recipe = { {ITEM_pine_wood, 2} } };
		items[ITEM_pine_wood] = (ItemData){ .pretty_name = STR("Pine Wood"), .for_structure = ARCH_workbench, .crafting_recipe={ {ITEM_pine_wood, 5}, {ITEM_rock, 1} } };
	}

	// :Spawn Entities

	// Spawn rocks
	for (int i = 0; i < 10; i++) 
	{
		Entity* en = entity_create();
		setup_rock(en);
		en -> pos = v2(i * 10.0, 0.0);
		en -> pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
		en -> pos = round_v2_to_tile(en -> pos);
		//en -> pos.y -= tile_width * 0.5;
	}

	// Spawn trees
	for (int i = 0; i < 10; i++) 
	{
		Entity* en = entity_create();
		setup_tree(en);
		en -> pos = v2(i * 10.0, 0.0);
		en -> pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
		en -> pos = round_v2_to_tile(en -> pos);
		//en -> pos.y -= tile_width * 0.5;
	}

	// :Ease of Testing

	// Start with X of X item for ease of testing
	{
		world -> inventory_items[ITEM_pine_wood].amount = 50;
		world -> inventory_items[ITEM_rock].amount = 50;
	}

	// X building starts placed in world for ease of testing
	{
		/*
		Entity* en = entity_create();

		setup_furnace(en);
		*/
	}

	// Spawn player
	Entity* player_en = entity_create();
	setup_player(player_en);

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
		last_time = current_time;

		// Log fps
		if ((int)current_time != (int)last_time) log("%.2f FPS\n%.2fms", 1.0 / (current_time - last_time), (current_time - last_time) * 1000);
		last_time = current_time;

		// :Frame Update

		draw_frame.enable_z_sorting = true;

		world_frame.world_proj = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		
		// Camera
		{
			Vector2 target_pos = player_en -> pos;
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

		// :Select entity

		if (!world_frame.hover_consumed)
		{
			// log("%f, %f", mouse_pos_world.x, mouse_pos_world.y);
			// draw_text(font, sprint(temp, STR("%f %f"), mouse_pos_world.x, mouse_pos_world.y), font_height, mouse_pos_world, v2(0.1, 0.1), COLOR_RED);

			float smallest_dist = INFINITY;

			for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
			{
				Entity* en = & world -> entities[i];

				if (en -> is_valid && (en -> destroyable_world_item || en -> workbench_thing)) 
				{
					SpriteData* sprite = get_sprite(en -> sprite_id);
					int entity_tile_x = world_pos_to_tile_pos(en -> pos.x);
					int entity_tile_y = world_pos_to_tile_pos(en -> pos.y);
					float dist = fabsf(v2_dist(en -> pos, mouse_pos_world));

					if (dist < entity_selection_radius) 
					{
						if (!world_frame.selected_entity || (dist < smallest_dist)) 
						{
							world_frame.selected_entity = en;
							smallest_dist = dist;
						}
					}
				}
			}
			//world space current location debug for mouse pos
			//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), mouse_pos_world.x, mouse_pos_world.y), font_height, mouse_pos_world, v2(0.1, 0.1), COLOR_RED);
		}

		// :Tile Rendering
		{
			int player_tile_x = world_pos_to_tile_pos(player_en -> pos.x);
			int player_tile_y = world_pos_to_tile_pos(player_en -> pos.y);
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

		// :Update Entities

		{
			for (int i = 0; i < MAX_ENTITY_COUNT; i++)
			{
				Entity* en = & world -> entities[i];

				if (en -> is_valid)
				{
					// switch (en -> arch) {
					// ...
					// }

					if (en -> workbench_thing) 
					{
						if (en -> current_crafting_item) 
						{
							float length = en -> current_crafting_item;
							if (en -> crafting_end_time == 0) 
							{
								en -> crafting_end_time = now() + length;
							}

							float alpha = alpha_from_end_time(en -> crafting_end_time, length);
							// log("%f", alpha);

							if (has_reached_end_time(en -> crafting_end_time)) 
							{
								// craft thing
								{
									Entity* item = entity_create();
									setup_item(item, en -> current_crafting_item);
									item -> pos = en -> pos;
								}
								en -> current_crafting_amount -= 1;
								en -> crafting_end_time = 0;

								assert(en -> current_crafting_amount >= 0, "fuckie wuckie");
								
								if (en -> current_crafting_amount == 0) 
								{
									en -> current_crafting_item = 0;
								}
							}
						}
					}

					if (en -> is_item)
					{
						// TODO epic physics pickup
						// Fabsf converts things to always be positive?
						if(fabsf(v2_dist(en -> pos, player_en -> pos)) < player_pickup_radius)
						{
							// Pickup item
							
							world -> inventory_items[en -> item].amount += 1;

							entity_destroy(en);
						}
					}
				}
			}
		}

		// :Selection
		{
			Entity* selected_en = world_frame.selected_entity;

			if (selected_en && selected_en -> destroyable_world_item)
			{
				if (is_key_just_pressed(MOUSE_BUTTON_LEFT))
				{
					consume_key_just_pressed(MOUSE_BUTTON_LEFT);
					
					selected_en -> health -= 1;

					if (selected_en -> health <= 0)
					{
						switch (selected_en -> arch)
						{
							case ARCH_tree:
							{
								Entity* en = entity_create();
								setup_item(en, ITEM_pine_wood);
								en -> pos = selected_en -> pos;
								break;
							}

							case ARCH_rock:
							{
								Entity* en = entity_create();
								setup_item(en, ITEM_rock);
								en -> pos = selected_en -> pos;
								break;
							}

							default:
							{
								break;
							}
						}

						entity_destroy(selected_en);
					}
				}
			}

			// :Interact
			if (selected_en && selected_en -> workbench_thing) 
			{
				if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) 
				{
					consume_key_just_pressed(MOUSE_BUTTON_LEFT);
					world -> ux_state = UX_workbench;
					world -> open_workbench = selected_en;
				}
			}
		}

		// :Render Entities

		for (int i = 0; i < MAX_ENTITY_COUNT; i++)
		{
			Entity* en = & world -> entities[i];

			if (en -> is_valid)
			{
				switch (en -> arch)
				{	
					/*
					case ARCH_player:
					{
						break;
					}
					*/ 

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

				// craft progress bar thing
				if (en -> current_crafting_item && en -> workbench_thing) 
				{
					float radius = 4.0; 

					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(en -> pos.x, en -> pos.y + 14.0, 0));
						xform = m4_translate(xform, v3(-radius, -radius, 0));
						draw_circle_xform(xform, v2(radius * 2, radius * 2), COLOR_WHITE);
					}

					ItemData craft_item_data = get_item_data(en -> current_crafting_item);

					float alpha = alpha_from_end_time(en -> crafting_end_time, en -> current_crafting_item);

					{
						Matrix4 xform = m4_identity;
						xform = m4_translate(xform, v3(en -> pos.x, en -> pos.y + 14.0, 0));
						xform = m4_scale(xform, v3(alpha, alpha, 1.0));
						xform = m4_translate(xform, v3(-radius, -radius, 0));
						draw_circle_xform(xform, v2(radius*2, radius*2), COLOR_BLACK);
					}
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

		player_en -> pos = v2_add(player_en -> pos, v2_mulf(input_axis, 100.0 * delta_t));

		os_update(); 
		gfx_update();
	}

	return 0;
}