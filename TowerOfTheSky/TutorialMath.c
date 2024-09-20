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

// :Tile 

const int tile_width = 16;

float tile_radius = 30.0; 

// :Utilities

float sin_breathe(float time, float rate)
{
	return (sin(time * rate) + 1.0 / 2.0);
}

int world_pos_to_tile_pos(float world_pos) 
{
	return roundf(world_pos / (float)tile_width);
}

float tile_pos_to_world_pos(int tile_pos) 
{
	return ((float)tile_pos * (float)tile_width);
}

Vector2i v2_world_pos_to_tile_pos(Vector2 world_pos) 
{
	return (Vector2i) { world_pos_to_tile_pos(world_pos.x), world_pos_to_tile_pos(world_pos.y) };
}

Vector2 v2_tile_pos_to_world_pos(Vector2i tile_pos) 
{
	return (Vector2) { tile_pos_to_world_pos(tile_pos.x), tile_pos_to_world_pos(tile_pos.y) };
}

Vector2 round_v2_to_tile(Vector2 world_pos) 
{
	world_pos.x = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.x));
	world_pos.y = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.y));
	return world_pos;
}
#define clamp_bottom(a, b) max(a, b)
#define clamp_top(a, b) min(a, b)

bool v4_equals(Vector4 a, Vector4 b) 
{
 return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool v2i_equals(Vector2i a, Vector2i b) 
{
 return a.x == b.x && a.y == b.y;
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

bool is_even(int number) 
{
    return number % 2 == 0;
}

// pad_pct just shrinks the rect by a % of itself ... 0.2 is a nice default

Draw_Quad* draw_sprite_in_rect(SpriteData sprite_data, Range2f rect, Vector4 col, float pad_pct) 
{
	Vector2 sprite_size = get_sprite_size(sprite_data);

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

		// height is a ratio of width
		Vector2 range_size = range2f_size(rect);
		rect.max.y = rect.min.y + (range_size.x * (sprite_size.y / sprite_size.x));
		// center along the Y
		float new_height = rect.max.y - rect.min.y;
		rect = range2f_shift(rect, v2(0, (range_size.y - new_height) * 0.5));

	} 
	else if (sprite_size.y > sprite_size.x) 
	{ 
		
		// width is a ratio of height
		Vector2 range_size = range2f_size(rect);
		rect.max.x = rect.min.x + (range_size.y * (sprite_size.x/sprite_size.y));
		// center along the X
		float new_width = rect.max.x - rect.min.x;
		rect = range2f_shift(rect, v2((range_size.x - new_width) * 0.5, 0));
	}

	return draw_image(sprite_data.image, rect.min, range2f_size(rect), col);
}