enum entity_type_e
{
    ENTITY_BALL,
    ENTITY_ENEMY,
};

enum rotation_type_e
{
    ROTATE_CW = 0,
    ROTATE_CCW,
};

typedef struct
{
    double red;
    double green;
    double blue;
    double alpha;
} color_t;

typedef struct
{
    v2 *spawn_loc; // TODO: maybe add support for multiple locations? (probably not needed)
    double spawn_rate;             // in milliseconds
    double time_since_last_spawn;  // in milliseconds
    int particles_per_spawn;
    double arc_center;
    double arc_size;
    rotation_type_e rotation_type;
    double angle_step;
    double last_angle; // TODO: remember to reset this once the attack is finished
    double particle_speed;
    const char *particle_image;
    int particle_width;
    int particle_height;
    v3 particle_color;
    // TODO: maybe add a pointer to function that can be called every in particle's update
} atk_pattern_t;

typedef struct
{
    bool stopped;
    bool forced_movement;
    v2 target_loc;
    v2 path[10];
    int path_size;

    //double cooldown; // this might not be needed (or helpful at all)
    double time_since_fight_started;

    // TODO: maybe change this a pointer?
    // NOTE: attacks do not necessarily happen in order, they can even happen simultaneously
    atk_pattern_t atks[10];
} enemy_only_t;

typedef struct
{
    v2 pos;
    double speed;
    // TODO: consider adding velocity and acceleration here

    // TODO: make this be only a path so that we don't need the renderer to update the game
    // TODO: make sure the renderer does not render the same image multiple times (hash_table)
    const char *image_path;
    int w;
    int h;

    entity_type_e type;
    union
    {
        enemy_only_t enemy_data;
    };
} entity_t;

typedef struct
{
    v2 pos;
    v2 velocity;
    v2 acceleration;

    // TODO: make this be only a path so that we don't need the renderer to update the game
    // TODO: make sure the renderer does not render the same image multiple times (hash_table)
    const char *image_path;
    int w;
    int h;
    v3 color;
} particle_t;

typedef struct
{
    color_t background_color;

    entity_t mouse_ball;
    entity_t keybd_ball;
    entity_t enemy;

    //particle_t particles[3000]; // TODO: create a linked list for this
    //int num_particles;
    std::list<particle_t> *particles;
} game_state_t;
