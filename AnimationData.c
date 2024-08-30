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

AnimationInfo create_animation_info(
    Gfx_Image *anim_sheet, 
    u32 anim_start_frame_x,
    u32 anim_start_frame_y,
    u32 anim_end_frame_x,
    u32 anim_end_frame_y,
    u32 number_of_columns,
    u32 number_of_rows
)
{
    assert(anim_sheet, "Invalid animation sheet provided");

    // Calculate frame dimensions
    u32 anim_frame_width  = anim_sheet -> width  / number_of_columns;
    u32 anim_frame_height = anim_sheet -> height / number_of_rows;

    // Calculate start and end indices in the sprite sheet
    u32 anim_start_index = anim_start_frame_y * number_of_columns + anim_start_frame_x;
    u32 anim_end_index = anim_end_frame_y * number_of_columns + anim_end_frame_x;
    u32 anim_number_of_frames = max(anim_end_index, anim_start_index) - min(anim_end_index, anim_start_index) + 1;

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

    // Return the fully initialized AnimationInfo structure
    AnimationInfo anim_info = 
    {
        .anim_sheet = anim_sheet,
        .number_of_columns = number_of_columns,
        .number_of_rows = number_of_rows,
        .anim_number_of_frames = anim_number_of_frames,
        .anim_frame_width = anim_frame_width,
        .anim_frame_height = anim_frame_height,
        .anim_duration = anim_duration,
        .anim_start_time = os_get_elapsed_seconds(),
        .anim_start_index = anim_start_index,
    };

    return anim_info;
}

void play_animation(AnimationInfo *anim_info, float32 current_time, Vector2 render_position)
{
    // Calculate the elapsed time for the animation
    float32 anim_elapsed = fmodf(current_time - anim_info -> anim_start_time, anim_info -> anim_duration);

    // Get the current progression in the animation from 0.0 to 1.0
    float32 anim_progression_factor = anim_elapsed / anim_info -> anim_duration;
    
    // Determine the current frame index in the animation sequence
    u32 anim_current_index = anim_info -> anim_number_of_frames * anim_progression_factor;
    u32 anim_absolute_index_in_sheet = anim_info -> anim_start_index + anim_current_index;
    
    // Calculate the position of the current frame in the sprite sheet
    u32 anim_index_x = anim_absolute_index_in_sheet % anim_info -> number_of_columns;
    u32 anim_index_y = anim_absolute_index_in_sheet / anim_info -> number_of_columns + 1;
    
    u32 anim_sheet_pos_x = anim_index_x * anim_info -> anim_frame_width;
    u32 anim_sheet_pos_y = (anim_info -> number_of_rows - anim_index_y) * anim_info -> anim_frame_height; // Y inverted
    
    // Draw the sprite sheet with the UV box for the current frame
    Draw_Quad *quad = draw_image(anim_info -> anim_sheet, v2(render_position.x, render_position.y), v2(anim_info -> anim_frame_width * 4, anim_info -> anim_frame_height * 4), COLOR_WHITE);
    quad -> uv.x1 = (float32)(anim_sheet_pos_x) / (float32)anim_info -> anim_sheet -> width;
    quad -> uv.y1 = (float32)(anim_sheet_pos_y) / (float32)anim_info -> anim_sheet -> height;
    quad -> uv.x2 = (float32)(anim_sheet_pos_x + anim_info -> anim_frame_width) / (float32)anim_info -> anim_sheet -> width;
    quad -> uv.y2 = (float32)(anim_sheet_pos_y + anim_info -> anim_frame_height) / (float32)anim_info -> anim_sheet -> height;

    /*
    // Uncomment to visualize the sprite sheet animation for debugging
    Vector2 sheet_pos = v2(0, 0);
    Vector2 sheet_size = v2(anim_info -> anim_sheet -> width, anim_info -> anim_sheet -> height);
    Vector2 frame_pos_in_sheet = v2(anim_sheet_pos_x, anim_sheet_pos_y);
    Vector2 frame_size = v2(anim_info -> anim_frame_width, anim_info -> anim_frame_height);
    draw_rect(sheet_pos, sheet_size, COLOR_BLACK); // Draw black background
    draw_rect(v2_add(sheet_pos, frame_pos_in_sheet), frame_size, COLOR_WHITE); // Draw white rect on current frame
    draw_image(anim_info -> anim_sheet, sheet_pos, sheet_size, COLOR_WHITE); // Draw the sheet
    */
}

inline float64 Animation_now() 
{
    return os_get_elapsed_seconds();
}

void update_animation(AnimationInfo *animation, Vector2 render_position)
{
    float32 current_time = Animation_now(); // Get the current time
    play_animation(animation, current_time, render_position); // Play the animation at the specified position
}

// Setup Fireball animation
AnimationInfo Fireball;

void setup_fireball_anim()
{
    Gfx_Image *anim_sheet = load_image_from_disk(STR("Resources/Sprites/fireball_sprite_sheet.png"), get_heap_allocator());
    assert(anim_sheet, "Could not open Resources/Sprites/fireball_sprite_sheet.png");
    
    Fireball = create_animation_info
    (
        // Sprite sheet setup for fireball
        anim_sheet,
        0,  // anim_start_frame_x
        0,  // anim_start_frame_y
        2,  // anim_end_frame_x
        0,  // anim_end_frame_y
        3,  // number_of_columns
        1   // number_of_rows
    );
}