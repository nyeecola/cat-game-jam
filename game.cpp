#include "game.hpp"

particle_t spawn_particle_towards(v2 pos, v2 vector, double speed, v2 acceleration, const char *image, int w, int h, v3 color)
{
    assert(image);

	// DEBUG
#if 0
	static int count = 0;
	printf("count = %d\n", ++count);
#endif

    v2 direction;
    if (!is_null_vector(vector)) direction = math_normalize(vector);
    else direction = vector;

    // TODO: maybe use malloc, since this is called so many times
    //       if so, remember to free
    particle_t particle = {};
    particle.pos = pos;
    particle.velocity = direction * speed;
    particle.acceleration = acceleration;
    particle.image_path = image;
    particle.w = w;
    particle.h = h;
    particle.color = color;

    return particle;
}

void update_particle_position(particle_t *particle, double dt)
{
    // TODO: think how to do this (change accelearation) (probably using pointer to function?)
    //v2 ortho = V2(-particle->velocity.y, particle->velocity.x) * 0.0005;
    //particle->acceleration = rotate_vector(ortho, -0.1f);

	// FIXME: this may be wrong (dt squared?)
    particle->velocity += particle->acceleration * dt;
    particle->pos += particle->velocity * dt;
}

// creates a circular attack using the state passed by `atk`
inline void do_circular_atk(std::list<particle_t> *particles, atk_pattern_t *atk, double dt)
{
    if (atk->time_since_last_spawn > atk->spawn_rate)
    {
        atk->time_since_last_spawn = atk->time_since_last_spawn - atk->spawn_rate + dt;

        double x, y;
        for (int i = 0; i < atk->particles_per_spawn; i++)
        {
            int step = atk->rotation_type == ROTATE_CW ? 1 : -1;

            double radians = ((atk->arc_size / atk->particles_per_spawn) * i * step) +
                             (atk->last_angle += atk->angle_step * step) +
                             atk->arc_center - atk->arc_size / 2;
            x = cos(radians);
            y = sin(radians);

            particle_t particle = spawn_particle_towards(*atk->spawn_loc,
                                                         V2(x, y),
                                                         atk->particle_speed,
                                                         atk->particle_accel,
                                                         atk->particle_image,
                                                         atk->particle_width,
                                                         atk->particle_height,
                                                         atk->particle_color);
            particles->push_back(particle);
        }
    }
    else
    {
        atk->time_since_last_spawn += dt;
    }
}

// does all ai actions
void ai_do_actions(game_state_t *game, double dt)
{
    assert(game);
    assert(dt > 0);

    // TODO: different behaviors
    //       probably each enemy should have different movement patterns (with a few exceptions)
    //       also, each enemy should have different attacks (maybe some few exceptions as well)

    // TODO: change this for a loop iterating over all ai controlled entities
    entity_t *enemy = &game->enemy;
    assert(enemy->type == ENTITY_ENEMY);

    // TODO: maybe consider movement an action? or something different?
    //       (find a way to only move from time to time)
    //       (also, it should be possible to synchronize movement with attacks)
    // movement
    {
        // if stopped, acquire new target location
        if (!enemy->enemy_data.forced_movement && enemy->enemy_data.stopped)
        {
            int new_target = rand() % enemy->enemy_data.path_size;
            enemy->enemy_data.target_loc = enemy->enemy_data.path[new_target];
            enemy->enemy_data.stopped = false;
        }

        // move to current target location
        v2 offset = enemy->enemy_data.target_loc - enemy->pos;
        double magnitude = math_magnitude(offset);
        v2 direction;
        if (!is_null_vector(offset)) direction = math_normalize(offset);
        else direction = offset;
        v2 velocity = direction * enemy->speed * dt;
        if (magnitude < enemy->speed * dt)
        {
            enemy->pos += offset;
            enemy->enemy_data.stopped = true;
        }
        else
        {
            enemy->pos += velocity;
        }
    }

    // attacks
    {
        if (enemy->enemy_data.time_since_fight_started > 5 &&
            enemy->enemy_data.time_since_fight_started < 8)
        {
            do_circular_atk(game->particles, &enemy->enemy_data.atks[0], dt);
        }

        if (enemy->enemy_data.time_since_fight_started > 2 &&
            enemy->enemy_data.time_since_fight_started < 9)
        {
            do_circular_atk(game->particles, &enemy->enemy_data.atks[1], dt);
            do_circular_atk(game->particles, &enemy->enemy_data.atks[2], dt);
        }

        if (enemy->enemy_data.time_since_fight_started > 10 &&
            enemy->enemy_data.time_since_fight_started < 20)
        {
            if (!enemy->enemy_data.forced_movement)
            {
                enemy->enemy_data.forced_movement = true;
                enemy->enemy_data.stopped = false;

                v2 loc = V2(300, 100);
                enemy->enemy_data.target_loc = loc;
            }

            if (enemy->enemy_data.stopped)
            {
                v2 player_dir = game->player.pos - enemy->pos;
                if (is_null_vector(player_dir))
                {
                    player_dir = math_normalize(player_dir);
                }
                double angle = atan2(player_dir.y, player_dir.x);
                enemy->enemy_data.atks[3].arc_center = angle;
                do_circular_atk(game->particles, &enemy->enemy_data.atks[3], dt);
            }
        }

        if (enemy->enemy_data.time_since_fight_started > 20)
        {
            enemy->enemy_data.forced_movement = false;
        }

        if (enemy->enemy_data.time_since_fight_started > 23 &&
            enemy->enemy_data.time_since_fight_started < 27)
        {
            do_circular_atk(game->particles, &enemy->enemy_data.atks[6], dt);
        }

        if (enemy->enemy_data.time_since_fight_started > 30 &&
            enemy->enemy_data.time_since_fight_started < 34)
        {
            do_circular_atk(game->particles, &enemy->enemy_data.atks[7], dt);
        }

        if (enemy->enemy_data.time_since_fight_started > 31 &&
            enemy->enemy_data.time_since_fight_started < 35)
        {
            do_circular_atk(game->particles, &enemy->enemy_data.atks[4], dt);
            do_circular_atk(game->particles, &enemy->enemy_data.atks[5], dt);
        }
    }

    enemy->enemy_data.time_since_fight_started += dt;

    // TODO: stop attacking once the fight is over (maybe destroy enemy or reset timer to reuse?)
}

game_state_t *game_state_initialize()
{
    game_state_t *game = (game_state_t *) calloc(1, sizeof(*game));
    game->particles = new std::list<particle_t>();

    // background_color
    game->background_color.red = 0;
    game->background_color.green = 85;
    game->background_color.blue = 127;
    game->background_color.alpha = 255;

    // keyboard controlled ball
    game->player = {};
    game->player.image_path = BALL_IMG_PATH;
    game->player.w = 30;
    game->player.h = 42;
    game->player.pos = V2(300, 700);
    game->player.speed = 200;

    // enemy
    {
        game->enemy = {};
        game->enemy.type = ENTITY_ENEMY;
        game->enemy.image_path = BALL_IMG_PATH;
        game->enemy.w = 100;
        game->enemy.h = 100;
        game->enemy.pos = V2(400, 60);
        game->enemy.speed = 60;
        game->enemy.enemy_data.time_since_fight_started = 0.0;
		game->enemy.health = 300;

        // movement
        {
            game->enemy.enemy_data.stopped = true;
            game->enemy.enemy_data.path[0] = V2(150, 150);
            game->enemy.enemy_data.path[1] = V2(500, 100);
            game->enemy.enemy_data.path[2] = V2(280, 200);
            game->enemy.enemy_data.path[3] = V2(30, 80);
            game->enemy.enemy_data.path[4] = V2(300, 120);
            game->enemy.enemy_data.path[5] = V2(530, 180);
            game->enemy.enemy_data.path_size = 6;
        }

        // attacks
        {
            // fast spiral from enemy
            {
                atk_pattern_t atk = {};
                atk.particle_image = BALL_IMG_PATH;
                atk.particle_width = 20;
                atk.particle_height = 20;
                atk.particle_color = V3(0, 0, 0);
                atk.spawn_loc = &game->enemy.pos;
                atk.spawn_rate = 0.20;
                atk.particles_per_spawn = 20;
                atk.arc_center = 0;
                atk.arc_size = PI * 2;
                atk.angle_step = 0.0;
                atk.last_angle = 0.0;
                atk.particle_speed = 250;
                atk.particle_accel = V2(0, 0);
                game->enemy.enemy_data.atks[0] = atk;
            }

            // two medium-to-fast spirals from the top corners of the screen
            // also, two fast circle cannons from the top corners of the screen
            {
                atk_pattern_t atk = {};

                atk.particle_image = BALL_IMG_PATH;
                atk.particle_width = 30;
                atk.particle_height = 30;
                atk.spawn_rate = 0.25;
                atk.particles_per_spawn = 12;
                atk.arc_center = 0;
                atk.arc_size = PI * 2;
                atk.angle_step = 0.006;
                atk.last_angle = 0.0;
                atk.particle_speed = 150;
                atk.particle_accel = V2(0, 0);

                atk.rotation_type = ROTATE_CW;
                atk.particle_color = V3(180, 180, 255);
                v2 *atk1_loc = (v2 *) malloc(sizeof(*atk1_loc));
                *atk1_loc = V2(50, 150);
                atk.spawn_loc = atk1_loc;
                game->enemy.enemy_data.atks[1] = atk;

                //atk.rotation_type = ROTATE_CCW;
                atk.particle_color = V3(255, 180, 180);
                v2 *atk2_loc = (v2 *) malloc(sizeof(*atk2_loc));
                *atk2_loc = V2(550, 150);
                atk.spawn_loc = atk2_loc;
                game->enemy.enemy_data.atks[2] = atk;

                atk.particle_speed = 70;

                atk.particle_accel = V2(66, 180);
                atk.particle_color = V3(180, 180, 255);
                v2 *atk4_loc = (v2 *) malloc(sizeof(*atk4_loc));
                *atk4_loc = V2(50, 150);
                atk.spawn_loc = atk4_loc;
                game->enemy.enemy_data.atks[4] = atk;

                atk.particle_accel = V2(-66, 180);
                atk.particle_color = V3(255, 180, 180);
                v2 *atk5_loc = (v2 *) malloc(sizeof(*atk5_loc));
                *atk5_loc = V2(550, 150);
                atk.spawn_loc = atk5_loc;
                game->enemy.enemy_data.atks[5] = atk;
            }

            // dense small arc
            {
                atk_pattern_t atk = {};
                atk.particle_image = BALL_IMG_PATH;
                atk.particle_width = 16;
                atk.particle_height = 22;
                atk.particle_color = V3(0, 255, 0);
                atk.spawn_loc = &game->enemy.pos;
                atk.spawn_rate = 0.1;
                atk.particles_per_spawn = 8;
                atk.arc_center = PI / 2 + ((PI / 16) / atk.particles_per_spawn) / 2;
                atk.arc_size = PI / 16 + (PI / 16) / atk.particles_per_spawn;
                atk.angle_step = 0.0;
                atk.last_angle = 0.0;
                atk.particle_speed = 500;
                atk.particle_accel = V2(0, 0);
                game->enemy.enemy_data.atks[3] = atk;
            }

            // dense slow divisor all angles
            {
                atk_pattern_t atk = {};
                atk.particle_image = BALL_IMG_PATH;
                atk.particle_width = 12;
                atk.particle_height = 12;
                atk.particle_color = V3(255, 0, 255);
                atk.spawn_loc = &game->enemy.pos;
                atk.spawn_rate = 0.06;
                atk.particles_per_spawn = 16;
                atk.arc_center = 0;
                atk.arc_size = PI * 2;
                atk.angle_step = 0.001;
                atk.last_angle = 0.0;
                atk.particle_speed = 85;
                atk.particle_accel = V2(0, 0);
                game->enemy.enemy_data.atks[6] = atk;
            }

            // fast spiral from enemy
            {
                atk_pattern_t atk = {};
                atk.particle_image = BALL_IMG_PATH;
                atk.particle_width = 20;
                atk.particle_height = 20;
                atk.particle_color = V3(0, 0, 0);
                atk.spawn_loc = &game->enemy.pos;
                atk.spawn_rate = 0.34;
                atk.particles_per_spawn = 14;
                atk.arc_center = 0;
                atk.arc_size = PI * 2;
                atk.angle_step = 0.0;
                atk.last_angle = 0.0;
                atk.particle_speed = 200;
                atk.particle_accel = V2(0, 0);
                game->enemy.enemy_data.atks[7] = atk;
            }

        }
    }

    return game;
}

void game_state_update(game_state_t *game, input_t *input, double dt)
{
    assert(game);
    assert(input);
    assert(dt > 0);

    // keyboard ball movement
    {
        double speed = game->player.speed;
        if (input->keys_pressed[SDL_SCANCODE_LSHIFT] || input->keys_pressed[SDL_SCANCODE_RSHIFT])
        {
            speed *= 0.6;
        }
        v2 offset_dir = {0, 0};
        if (input->keys_pressed[SDL_SCANCODE_UP])
        {
            offset_dir.y = -1;
        }
        if (input->keys_pressed[SDL_SCANCODE_DOWN])
        {
            offset_dir.y = 1;
        }
        if (input->keys_pressed[SDL_SCANCODE_LEFT])
        {
            offset_dir.x = -1;
        }
        if (input->keys_pressed[SDL_SCANCODE_RIGHT])
        {
            offset_dir.x = 1;
        }
        if (!is_null_vector(offset_dir))
        {
            offset_dir = math_normalize(offset_dir);
        }
        game->player.pos += offset_dir * speed * dt;

        if (game->player.pos.x < 0)
        {
            game->player.pos.x = 0;
        }
        if (game->player.pos.y < 0)
        {
            game->player.pos.y = 0;
        }
        if (game->player.pos.x > DEFAULT_SCREEN_WIDTH)
        {
            game->player.pos.x = DEFAULT_SCREEN_WIDTH;
        }
        if (game->player.pos.y > DEFAULT_SCREEN_HEIGHT)
        {
            game->player.pos.y = DEFAULT_SCREEN_HEIGHT;
        }
    }

    // perform enemy's actions
    ai_do_actions(game, dt);

    // update particles
    std::list<particle_t>::iterator it, end;
    for (it = game->particles->begin(), end = game->particles->end(); it != end;)
    {
        particle_t *particle = &(*it);

        update_particle_position(particle, dt);

        // TODO: use current instead of default (we don't have current yet :()
        if (particle->pos.x < -DEFAULT_SCREEN_WIDTH / 3 ||
            particle->pos.x > DEFAULT_SCREEN_WIDTH + DEFAULT_SCREEN_WIDTH / 3 ||
            particle->pos.y < -DEFAULT_SCREEN_HEIGHT / 3 ||
            particle->pos.y > DEFAULT_SCREEN_HEIGHT + DEFAULT_SCREEN_HEIGHT / 3)
        {
            it = game->particles->erase(it);
            continue;
        }

        ++it;
    }

    // update background
    {
        double multiplier = 100;
        static bool red_decreasing = false;
        if (red_decreasing)
        {
            game->background_color.red -= multiplier * dt;
            if (game->background_color.red < 5) red_decreasing = false;
        }
        else
        {
            game->background_color.red += multiplier * dt;
            if (game->background_color.red > 250) red_decreasing = true;
        }
        static bool blue_decreasing = false;
        if (blue_decreasing)
        {
            game->background_color.blue -= multiplier * dt;
            if (game->background_color.blue < 5) blue_decreasing = false;
        }
        else
        {
            game->background_color.blue += multiplier * dt;
            if (game->background_color.blue > 250) blue_decreasing = true;
        }
    }
}

void game_state_render(game_state_t *game, renderer_t *renderer)
{
    assert(game);
    assert(renderer);
    assert(renderer->sdl);

    // render background
    SDL_SetRenderDrawColor(renderer->sdl,
                           (u8) round(game->background_color.red),
                           (u8) round(game->background_color.green),
                           (u8) round(game->background_color.blue),
                           (u8) round(game->background_color.alpha));
    SDL_RenderClear(renderer->sdl);

    // declare rect that will store the render destination (on screen)
	SDL_Rect rect;

	// render player
	rect.x = (int) round(game->player.pos.x - game->player.w / 2);
	rect.y = (int) round(game->player.pos.y - game->player.h / 2);
	rect.w = (int) round(game->player.w);
	rect.h = (int) round(game->player.h);
	display_image(renderer, game->player.image_path, &rect, 0, V3(255, 255, 255));

	// render enemy
	rect.x = (int) round(game->enemy.pos.x - game->enemy.w / 2);
	rect.y = (int) round(game->enemy.pos.y - game->enemy.h / 2);
	rect.w = (int) round(game->enemy.w);
	rect.h = (int) round(game->enemy.h);
	display_image(renderer, game->enemy.image_path, &rect, 0, V3(255, 255, 255));

	// render particles
	std::list<particle_t>::iterator it, end;
	for (it = game->particles->begin(), end = game->particles->end(); it != end; ++it)
	{
		particle_t *particle = &(*it);

		rect.x = (int) round(particle->pos.x - particle->w / 2);
		rect.y = (int) round(particle->pos.y - particle->h / 2);
		rect.w = (int) round(particle->w);
		rect.h = (int) round(particle->h);

		display_image(renderer, particle->image_path, &rect, 0, particle->color);
	}

    // render enemy health
    int enemy_hearts = (int) ceil((double) game->enemy.health / HP_PER_HEART);
    for (int i = 1; i <= enemy_hearts; i++)
    {
        // destination rect
        rect.w = HP_UNIT_SIZE;
        rect.h = HP_UNIT_SIZE;
        // TODO: change to current screen width
        rect.x = DEFAULT_SCREEN_WIDTH - HP_UNIT_SIZE * i;
        rect.y = ENEMY_HP_BAR_Y;

        // render only part of the last heart if current hp is not a multiple of 100
        int hp_in_heart = game->enemy.health - HP_PER_HEART * (i - 1);
        if (hp_in_heart > HP_PER_HEART)
        {
            display_image(renderer, HP_IMG_PATH, &rect, 0, V3(255, 255, 255));
        }
        else
        {
            SDL_Rect source = {};
            double percentage_to_fill = ((double) hp_in_heart / HP_PER_HEART) * HP_IMG_SIZE;
            source.w = (int) percentage_to_fill;
            source.h = (int) HP_IMG_SIZE;
            source.x = HP_IMG_SIZE - source.w;
            rect.x += HP_IMG_SIZE - source.w;
            rect.w = source.w;
            display_image(renderer, HP_IMG_PATH, &rect, &source, V3(255, 255, 255));
        }
    }
}
