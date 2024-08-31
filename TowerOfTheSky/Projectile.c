#define MAX_PROJECTILES 100

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
};

Projectile projectiles[MAX_PROJECTILES];

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

void spawn_projectile(Vector2 spawn_position, Vector2 target_position, float speed, float damage, AnimationInfo *animation, float32 scale) 
{
    for (int i = 0; i < MAX_PROJECTILES; i++) 
    {
        if (!projectiles[i].is_active) 
        {
            // Initialize and add the projectile to the first available inactive slot
            Projectile *projectile = & projectiles[i];
            projectile -> is_active = true;
            projectile -> position = spawn_position;
            projectile -> target_position = target_position;
            projectile -> speed = speed;
            projectile -> damage = damage;
            projectile -> animation = *animation;
            projectile -> scale = scale;

            // Calculate the direction vector
            Vector2 direction = v2_sub(target_position, spawn_position);
            float32 length = v2_length(direction);
            
            // Normalize the direction vector
            if (length != 0.0f)
            {
                direction = v2_scale(direction, 1.0f / length);
            }

            // Calculate velocity
            projectile -> velocity = v2_scale(direction, speed);

            // Calculate rotation in degrees based on the direction
            projectile -> rotation = atan2f(-direction.y, direction.x) * (180.0f / M_PI);

            // Optionally, ensure the rotation is within [0, 360) range
            if (projectile -> rotation < 0.0f)
            {
                projectile -> rotation += 360.0f;
            }

            //log("Spawned projectile at position (%f, %f) targeting (%f, %f)\n", spawn_position.x, spawn_position.y, target_position.x, target_position.y);
            //log("Projectile active: %i", projectile -> is_active);

            break; // Exit the loop after adding the projectile
        }
    }
}

void update_projectile(Projectile *projectile, float delta_time) 
{
    if (!projectile -> is_active) return;

    // Update the position based on the velocity
    projectile -> position = v2_add(projectile -> position, v2_scale(projectile -> velocity, delta_time));

    // Check if the projectile has reached the target
    if (v2_distance(projectile -> position, projectile -> target_position) < projectile -> speed * delta_time) 
    {
        projectile -> position = projectile -> target_position; // Ensure it reaches the target
        projectile -> is_active = false;  // Deactivate the projectile
    }

    update_animation(& projectile -> animation, & projectile -> position, projectile -> scale, & projectile -> rotation);

    //log("Updating projectile at (%f, %f)\n", projectile -> position.x, projectile -> position.y);
}

Projectile fireball;