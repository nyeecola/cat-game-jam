// --------------------
// Math data
// --------------------

struct v2 {
    double x;
    double y;
};

struct v3 {
    double x;
    double y;
    double z;
};

struct seg_t{
    v2 a;
    v2 b;
};

// ---------------------
// Input data
// ---------------------

struct mouse_t {
    int x;
    int y;
};

struct input_t {
    // keyboard
    const uint8_t *keys_pressed; // this stores the state of each key and is always updated

    // mouse
    mouse_t mouse;
};

// --------------------
// Basic data
// --------------------

struct color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// ---------------------
// Game data
// ---------------------

// entity types
enum entity_type_e {
    ENEMY,
    PLAYER,
};

enum game_mode_e {
    PAUSED,
    PLAYING,
    START_SCREEN,
};

// enemy specific data
struct enemy_t {
    v2 *path;
    int path_len;
    int path_cur;
    float rotation_speed;

    v2 possibleDestinations[10];
    int possibleDestinations_len;
};

struct player_t {
    bool has_password;
};

// entity generic data 
struct entity_t {
    v2 previous_pos; // used to calculate rotation
    v2 pos;
    double speed;
    double angle;

    SDL_Texture *image;
    SDL_Texture *image2;
    int image_w;
    int image_h;

    int hitbox_r;

    entity_type_e type;
    union {
        enemy_t enemy_data; // this should be more specific than enemy (it is just an example)
        player_t player_data;
        // ... other entities data (i.e. shopkeeper specific data)
    };
};

enum tile_type_e {
    EMPTY,
    WALL,
    DOOR,
    LOCK,
    LASER_SOURCE,
    PASSWORD
};

struct door_t {
    int x;
    int y;
    int exit_x;
    int exit_y;
    int target_map;
    int target_door;
};

struct map_t {
    int w, h;
    int doors;

    door_t door[MAX_DOOR_PER_ROOM];
    tile_type_e tile[30][40];
    seg_t *hitbox; // all static line segments used for collision
    int hitbox_size;

    entity_t enemies[30];
    int enemies_count;

    SDL_Texture *wall_sprite;
    SDL_Texture *floor_sprite;
    SDL_Texture *doorw_sprite;
    SDL_Texture *doorh_sprite;
    SDL_Texture *lockw_sprite;
    SDL_Texture *lockh_sprite;
    SDL_Texture *password_sprite;
    SDL_Texture *laser_source_sprite;
};

// game state data
struct game_state_t {
    // current game mode
    game_mode_e game_mode;

    // map
    int current_map_id;
    int maps;
    map_t *map;

    // player
    entity_t player;
    v2 spawn_loc;
    int spawn_map_id;

    // scene data
    color_t background_color;

    // finale
    bool finale;
    double finale_timer;

    // music
    Mix_Music *music;
};
