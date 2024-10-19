typedef struct LightSource LightSource;

struct LightSource 
{
    Vector4 color;      // 16 bytes (4 floats)
    Vector2 position;   // 8 bytes (2 floats) + 8 bytes padding = 16 bytes
    Vector2 size;       // 8 bytes (2 floats) + 8 bytes padding = 16 bytes
    Vector2 direction;  // 8 bytes (2 floats) + 8 bytes padding = 16 bytes
    float intensity;    // 4 bytes
    float radius;       // 4 bytes
};