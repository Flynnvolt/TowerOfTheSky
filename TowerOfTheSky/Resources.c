typedef struct Resource Resource;

struct Resource 
{
    bool unlocked;
    float current;
    float max;
    float per_second;
};

// Resources
Resource mana = {true, 0.0, 100.0, 10.0};
Resource intellect = {false, 0.0, 50.0, 0.5};