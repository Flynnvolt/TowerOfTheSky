#pragma once

typedef struct AnimationInfo AnimationInfo;

struct AnimationInfo
{  
    Gfx_Image *anim_sheet;
    u32 number_of_columns;
    u32 number_of_rows;
    u32 anim_number_of_frames;
    u32 anim_frame_width;
    u32 anim_frame_height;
    u32 anim_start_index;
    float32 anim_duration;
    float32 anim_start_time;
    float32 playback_fps;
};

AnimationInfo create_animation_info
(
    Gfx_Image *anim_sheet, 
    u32 anim_start_frame_x,
    u32 anim_start_frame_y,
    u32 anim_end_frame_x,
    u32 anim_end_frame_y,
    u32 number_of_columns,
    u32 number_of_rows,
    float32 playback_fps
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

inline float32 degrees_to_radians(float32 degrees) 
{
    return degrees * (M_PI / 180.0f);
}

void play_animation(AnimationInfo *anim_info, float32 current_time, Vector2 *position, float32 scale_ratio, float32 *rotation_degrees, Draw_Frame *frame) 
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
    u32 anim_index_y = anim_absolute_index_in_sheet / anim_info -> number_of_columns;
    
    u32 anim_sheet_pos_x = anim_index_x * anim_info -> anim_frame_width;
    u32 anim_sheet_pos_y = (anim_info -> number_of_rows - anim_index_y - 1) * anim_info -> anim_frame_height; // Y inverted

    // Scale factor and size calculation
    float32 frame_width_scaled = anim_info -> anim_frame_width * scale_ratio;
    float32 frame_height_scaled = anim_info -> anim_frame_height * scale_ratio;

    // Ensure pixel-perfect scaling
    frame_width_scaled = roundf(frame_width_scaled);
    frame_height_scaled = roundf(frame_height_scaled);

    // Convert rotation from degrees to radians
    float32 rotation_radians = degrees_to_radians(*rotation_degrees);

    // Create rotation matrix
    Matrix4 rotation_matrix = m4_rotate_z(m4_identity(), rotation_radians);

    // Translate the sprite to its center for correct pivoting
    Vector3 center_translation = { -frame_width_scaled / 2.0f, -frame_height_scaled / 2.0f, 0.0f };
    Matrix4 center_translation_matrix = m4_translate(m4_identity(), center_translation);

    // Create a translation matrix for the sprite's position
    Vector3 position_3d = { position -> x, position -> y, 0.0f };
    Matrix4 translation_matrix = m4_translate(m4_identity(), position_3d);

    // Combine center translation, rotation, and position translation into a single transformation matrix
    Matrix4 transformation_matrix = m4_mul(translation_matrix, m4_mul(rotation_matrix, center_translation_matrix));

    // Temp fix for GPU driver issue
    float64 px_width  = 1.0 / (float64)anim_info -> anim_sheet -> width;
    float64 px_height = 1.0 / (float64)anim_info -> anim_sheet -> height;

    // Draw the sprite sheet with the UV box for the current frame
    Draw_Quad *quad = draw_image_xform_in_frame(anim_info -> anim_sheet, transformation_matrix, v2(frame_width_scaled, frame_height_scaled), COLOR_WHITE, frame);
    quad -> uv.x1 = (float32)(anim_sheet_pos_x) / (float32)anim_info -> anim_sheet -> width + (float32)px_width * 0.1f;
    quad -> uv.y1 = (float32)(anim_sheet_pos_y) / (float32)anim_info -> anim_sheet -> height + (float32)px_height * 0.1f;
    quad -> uv.x2 = (float32)(anim_sheet_pos_x + anim_info -> anim_frame_width) / (float32)anim_info -> anim_sheet -> width;
    quad -> uv.y2 = (float32)(anim_sheet_pos_y + anim_info -> anim_frame_height) / (float32)anim_info -> anim_sheet -> height;
}

inline float64 Animation_now() 
{
    return os_get_elapsed_seconds();
}

void update_animation(AnimationInfo *animation, Vector2 *position, float32 scale_ratio, float32 *rotation_degrees, Draw_Frame *frame) 
{
    float32 current_time = Animation_now(); // Get the current time
    play_animation(animation, current_time, position, scale_ratio, rotation_degrees, frame); // Play the animation
}

// Setup Fireball animation
AnimationInfo Fireball;

void setup_fireball_anim(const string base_path)
{
    string full_path;
    full_path = string_concat(base_path, STR("\\Resources\\Sprites\\fireball_sprite_sheet.png"), get_temporary_allocator());

    Gfx_Image *anim_sheet = load_image_from_disk(full_path, get_heap_allocator());
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
        1,  // number_of_rows
        16  // playback_fps
    );
}