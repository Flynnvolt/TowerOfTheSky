typedef struct AnimationInfo AnimationInfo;

struct AnimationInfo
{  
    Gfx_Image *anim_sheet;
    u32 number_of_columns;
    u32 number_of_rows;
    u32 anim_number_of_frames;
    u32 anim_frame_width;
    u32 anim_frame_height;
    float32 anim_duration;
    float32 anim_start_time;
    u32 anim_start_index;
};

AnimationInfo Fireball = 
{
    .anim_sheet = null,
    .number_of_columns = null,
    .number_of_rows = null,
    .anim_number_of_frames = null,
    .anim_frame_width = null,
    .anim_frame_height = null,
    .anim_duration = null,
    .anim_start_time = null,
    .anim_start_index = null,
};

// Fireball Animation setup test
void setup_fireball_anim()
{
	Gfx_Image *anim_sheet = load_image_from_disk(STR("Resources/Sprites/fireball_sprite_sheet.png"), get_heap_allocator());
	assert(anim_sheet, "Could not open Resources/Sprites/fireball_sprite_sheet.png");
	
	// Configure information about the whole image as a sprite sheet
	u32 number_of_columns = 3;
	u32 number_of_rows = 1;
	u32 total_number_of_frames = number_of_rows * number_of_columns;
	
	u32 anim_frame_width  = anim_sheet -> width  / number_of_columns;
	u32 anim_frame_height = anim_sheet -> height / number_of_rows;
	
	// Configure the animation by setting the start & end frames in the grid of frames
	// (Inspect sheet image and count the frame indices you want)
	// In sprite sheet animations, it usually goes down. So Y 0 is actuall the top of the
	// sprite sheet, and +Y is down on the sprite sheet.
	u32 anim_start_frame_x = 0;
	u32 anim_start_frame_y = 0;
	u32 anim_end_frame_x = 2;
	u32 anim_end_frame_y = 0;
	u32 anim_start_index = anim_start_frame_y * number_of_columns + anim_start_frame_x;
	u32 anim_end_index   = anim_end_frame_y   * number_of_columns + anim_end_frame_x;
	u32 anim_number_of_frames = max(anim_end_index, anim_start_index)-min(anim_end_index, anim_start_index)+1;
	
	// Sanity check configuration
	assert(anim_end_index > anim_start_index, "The last frame must come before the first frame");
	assert(anim_start_frame_x < number_of_columns, "anim_start_frame_x is out of bounds");
	assert(anim_start_frame_y < number_of_rows, "anim_start_frame_y is out of bounds");
	assert(anim_end_frame_x < number_of_columns, "anim_end_frame_x is out of bounds");
	assert(anim_end_frame_y < number_of_rows, "anim_end_frame_y is out of bounds");
	
	// Calculate duration per frame in seconds
	float32 playback_fps = 4;
	float32 anim_time_per_frame = 1.0 / playback_fps;
	float32 anim_duration = anim_time_per_frame * (float32)anim_number_of_frames;

    // Fill out the Fireball structure
    Fireball.anim_sheet = anim_sheet;
    Fireball.number_of_columns = number_of_columns;
    Fireball.number_of_rows = number_of_rows;
    Fireball.anim_number_of_frames = anim_number_of_frames;
    Fireball.anim_frame_width = anim_frame_width;
    Fireball.anim_frame_height = anim_frame_height;
    Fireball.anim_duration = anim_duration;
    Fireball.anim_start_index = anim_start_index;
}