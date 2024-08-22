//added from tutorial 5 from randy
inline float v2_dist(Vector2 a, Vector2 b) {
    return v2_length(v2_sub(a, b));
}


const int tile_width = 16;

const float entity_selection_radius = 8.0f;

const int player_health = 10;

const int rock_health = 3;

const int tree_health = 3;

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
		return true; // reached
	}
	return false;
}

void animate_v2_to_target(Vector2* value, Vector2 target, float delta_t, float rate) 
{
	animate_f32_to_target(&(value->x), target.x, delta_t, rate);
	animate_f32_to_target(&(value->y), target.y, delta_t, rate);
}

typedef struct Sprite Sprite;

struct Sprite
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
	SPRITE_MAX,
};

Sprite sprites[SPRITE_MAX];

Sprite* get_sprite(SpriteID id)
{
	if (id >= 0 && id < SPRITE_MAX)
	{
		return & sprites[id];
	}
	return & sprites[0];
}

Vector2 get_sprite_size(Sprite* sprite)
{
	return (Vector2) {sprite -> image -> width, sprite -> image -> height};
}

typedef struct Item Item;

struct Item
{

};

typedef enum ItemID ItemID;

enum ItemID
{
	ITEM_nil,
	ITEM_rock,
	ITEM_pine_wood,
	ITEM_MAX,
};

Item items[ITEM_MAX];

Item* get_item(ItemID id)
{
	if (id >= 0 && id < ITEM_MAX)
	{
		return & items[id];
	}
	return & items[0];
}

typedef enum EntityArchetype EntityArchetype;

enum EntityArchetype
{
	arch_nil = 0,
	arch_player = 1,
	arch_item = 2,
	arch_rock = 3,
	arch_tree = 4,
	arch_tree2 = 5,
	ARCH_MAX,
};

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
	bool destroyable_world_item;
};

#define MAX_ENTITY_COUNT 1024

typedef struct World World;

struct World
{
	Entity entities[MAX_ENTITY_COUNT];
};

World* world = 0;

typedef struct WorldFrame WorldFrame;

struct WorldFrame 
{
	Entity* selected_entity;
};

WorldFrame world_frame;

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
	en -> arch = arch_player;
	en -> sprite_id = SPRITE_player;
	en -> health = player_health;
}

void setup_rock(Entity* en) 
{
	en -> arch = arch_rock;
	en -> sprite_id = SPRITE_rock0;
	en -> health = rock_health;
	en -> destroyable_world_item = true;
}

void setup_tree(Entity* en) 
{
	en -> arch = arch_tree;
	en -> sprite_id = SPRITE_tree0;
	en -> health = tree_health;
	en -> destroyable_world_item = true;
}

void setup_item_pine_wood(Entity* en)
{
	en -> item = ITEM_pine_wood;
	en -> sprite_id = SPRITE_item_pine_wood;
}

void setup_item_rock(Entity* en)
{
	en -> item = ITEM_rock;
	en -> sprite_id = SPRITE_item_gold;
}

Vector2 screen_to_world() 
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

int entry(int argc, char **argv) 
{
	window.title = STR("Tower of the Sky");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x2a2d3aff);

	world = alloc(get_heap_allocator(), sizeof(World));

	float64 last_time = os_get_elapsed_seconds();

	sprites[SPRITE_player] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/player.png"), get_heap_allocator())};
	sprites[SPRITE_tree0] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/tree0.png"), get_heap_allocator())};
	sprites[SPRITE_tree1] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/tree1.png"), get_heap_allocator())};
	sprites[SPRITE_rock0] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/rock0.png"), get_heap_allocator())};
	sprites[SPRITE_rock1] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/rock1.png"), get_heap_allocator())};
 	sprites[SPRITE_item_gold] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/gold.png"), get_heap_allocator())};
	sprites[SPRITE_item_pine_wood] = (Sprite){ .image = load_image_from_disk(STR("Resources/Sprites/pine_wood.png"), get_heap_allocator())};

	Gfx_Font* font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	const u32 font_height = 48;

	//spawn rocks
	for (int i = 0; i < 10; i++) 
	{
		Entity* en = entity_create();
		setup_rock(en);
		en -> pos = v2(i * 10.0, 0.0);
		en -> pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
		en -> pos = round_v2_to_tile(en -> pos);
		//en -> pos.y -= tile_width * 0.5;
	}

	//spawn trees
	for (int i = 0; i < 10; i++) 
	{
		Entity* en = entity_create();
		setup_tree(en);
		en -> pos = v2(i * 10.0, 0.0);
		en -> pos = v2(get_random_float32_in_range(-200, 200), get_random_float32_in_range(-200, 200));
		en -> pos = round_v2_to_tile(en->pos);
		//en -> pos.y -= tile_width * 0.5;
	}

	//spawn player
	Entity* player_en = entity_create();
	setup_player(player_en);

	float zoom = 3;
	Vector2 camera_pos = v2(0, 0);

	//Game Loop
	while (!window.should_close) 
	{
		reset_temporary_storage();
		world_frame = (WorldFrame){0};

		//time tracking
		float64 now = os_get_elapsed_seconds();
		float64 delta_t = now - last_time;

		//log fps
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		last_time = now;

		//scaling / zoom
		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		
		//camera
		{
			Vector2 target_pos = player_en -> pos;
			animate_v2_to_target(& camera_pos, target_pos, delta_t, 15.0f);

			draw_frame.camera_xform = m4_make_scale(v3(1.0, 1.0, 1.0));
			draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0.0)));
			draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_scale(v3(1.0/zoom, 1.0/zoom, 1.0)));
		}

		Vector2 mouse_pos_world = screen_to_world();
		int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
		int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);

		//tile rendering 
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
			//show which tile is currently selected
			//draw_rect(v2(tile_pos_to_world_pos(mouse_tile_x) + tile_width * -0.5, tile_pos_to_world_pos(mouse_tile_y) + tile_width * -0.5), v2(tile_width, tile_width), /*v4(0.5, 0.5, 0.5, 0.5)*/ v4(0.5, 0.0, 0.0, 1.0));
		}

		//mouse pos test
		float smallest_dist = INFINITY;

		{
			for (int i = 0; i < MAX_ENTITY_COUNT; i++) 
			{
				Entity* en = & world -> entities[i];
				if (en -> is_valid && en -> destroyable_world_item == true) 
				{
					Sprite* sprite = get_sprite(en -> sprite_id);

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
					}; 

					/*
					Range2f bounds = range2f_make_bottom_center(sprite->size);
					bounds = range2f_shift(bounds, en->pos);
					Vector4 col = COLOR_RED;
					col.a = 0.4;

					if (range2f_contains(bounds, mouse_pos_world)) 
					{
						draw_rect(bounds.min, range2f_size(bounds), col);
					}
					*/
				}
			}
			//world space current location debug for mouse pos
			//draw_text(font, sprint(get_temporary_allocator(), STR("%f %f"), mouse_pos_world.x, mouse_pos_world.y), font_height, mouse_pos_world, v2(0.1, 0.1), COLOR_RED);
		}

		//click thing
		{
			Entity* selected_en = world_frame.selected_entity;

			if (is_key_just_pressed(MOUSE_BUTTON_LEFT))
			{
				consume_key_just_pressed(MOUSE_BUTTON_LEFT);

				if (selected_en)
				{
					selected_en -> health -= 1;

					if (selected_en -> health <= 0)
					{
						switch (selected_en -> arch)
						{
							case arch_tree:
							{
								Entity* en = entity_create();
								setup_item_pine_wood(en);
								en -> pos = selected_en -> pos;
								break;
							}

							case arch_rock:
							{
								Entity* en = entity_create();
								setup_item_rock(en);
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
		}

		//render
		for (int i = 0; i < MAX_ENTITY_COUNT; i++)
		{
			Entity* en = & world -> entities[i];

			if (en -> is_valid)
			{
				switch (en -> arch)
				{	
					/*
					case arch_player:
					{
						break;
					}
					*/ 

					default:
					{
						Sprite* sprite = get_sprite(en -> sprite_id);
						Matrix4 xform = m4_scalar(1.0);
						xform         = m4_translate(xform, v3(0, tile_width * -0.5, 0));
						xform         = m4_translate(xform, v3(en -> pos.x, en -> pos.y, 0));
						xform         = m4_translate(xform, v3(sprite -> image -> width * -0.5, 0.0, 0));
						
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

		//Press Esc to Close Game
		if(is_key_just_pressed(KEY_ESCAPE))
		{
			window.should_close = true;
		}

		//Wasd Movement
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