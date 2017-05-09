void game_state_update(game_state_t *game_state, input_t *input, double dt)
{
    assert(game_state);
    assert(input);
    assert(dt > 0);

    if (game_state->type == GAME_MODE)
    {
        game_mode_t *game = game_state->game;
        assert(game);

        // player's actions movement
        // TODO: remove this check since we'll handle death differently
        if (game->player.health > 0)
        {
            do_players_actions(game, input, dt);
        }

        // perform enemy's actions
        ai_do_actions(game, dt);

        // update particles
        std::list<particle_t *>::iterator it, end;
        for (it = game->particles->begin(), end = game->particles->end(); it != end;)
        {
            particle_t *particle = *it;

            update_particle_position(particle, dt);

            // TODO: use current instead of default (we don't have current yet :()
            if (particle->pos.x < -DEFAULT_SCREEN_WIDTH / 3 ||
                particle->pos.x > DEFAULT_SCREEN_WIDTH + DEFAULT_SCREEN_WIDTH / 3 ||
                particle->pos.y < -DEFAULT_SCREEN_HEIGHT / 3 ||
                particle->pos.y > DEFAULT_SCREEN_HEIGHT + DEFAULT_SCREEN_HEIGHT / 3)
            {
                it = game->particles->erase(it);
                free(particle);
                continue;
            }

            if (detect_particle_collision(game, particle))
            {
                it = game->particles->erase(it);
                free(particle);

#if 1
                if (game->player.health <= 0)
                {
                    game_state->game = reset_game(game_state->game);
                    return;
                }
#endif

                continue;
            }

            ++it;
        }

        // update background
        {
            double multiplier = 40;
            static bool blue_decreasing = false;
            if (blue_decreasing)
            {
                game->background_color.blue -= multiplier * dt;
                if (game->background_color.blue < 120) blue_decreasing = false;
            }
            else
            {
                game->background_color.blue += multiplier * dt;
                if (game->background_color.blue > 250) blue_decreasing = true;
            }
        }
    }
    else if (game_state->type == MENU_MODE)
    {
        assert(game_state->menu);
    }
}

void game_state_render(game_state_t *game_state, renderer_t *renderer)
{
    assert(game_state);
    assert(renderer);
    assert(renderer->sdl);

    if (game_state->type == GAME_MODE)
    {
        assert(game_state->game);
        game_mode_t *game = game_state->game;

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
        // TODO: remove this check since we'll handle death differently
        if (game->player.health > 0)
        {
            rect.x = (int) round(game->player.pos.x - game->player.w / 2);
            rect.y = (int) round(game->player.pos.y - game->player.h / 2);
            rect.w = (int) round(game->player.w);
            rect.h = (int) round(game->player.h);
            display_image(renderer, game->player.image_path, &rect, 0, V3(255, 255, 255));
        }

        // render enemy
        rect.x = (int) round(game->enemy.pos.x - game->enemy.w / 2);
        rect.y = (int) round(game->enemy.pos.y - game->enemy.h / 2);
        rect.w = (int) round(game->enemy.w);
        rect.h = (int) round(game->enemy.h);
        display_image(renderer, game->enemy.image_path, &rect, 0, V3(255, 255, 255));

        // render particles
        std::list<particle_t *>::iterator it, end;
        for (it = game->particles->begin(), end = game->particles->end(); it != end; ++it)
        {
            particle_t *particle = *it;

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

        // render player health
        rect.w = HP_UNIT_SIZE;
        rect.h = HP_UNIT_SIZE;
        // TODO: change to current screen width
        rect.x = DEFAULT_SCREEN_WIDTH - HP_UNIT_SIZE;
        rect.y = PLAYER_HP_BAR_Y;
        display_image(renderer, HP_IMG_PATH, &rect, 0, V3(255, 0, 0));
    }
    else if (game_state->type == MENU_MODE)
    {
        assert(game_state->menu);
    }
}
