#include "jumper.h"
t_obj init_player();
t_obj init_spear();

void	getout(const char *s)
{
	size_t	i;
	char	*c;

	if (s != NULL)
	{
		c = "\n";
		i = 0;
		while (s[i] != '\0')
			i++;
		write(2, s, i);
		write(2, c, 1);
	}
	exit(EXIT_FAILURE);
}

static void	init(t_rend *renderer, t_jump *jump)
{
	bzero(jump, sizeof(t_jump));

	bzero(renderer, sizeof(t_rend));
	renderer->win_buffer = (t_buffer *)malloc(sizeof(t_buffer));
	if (!renderer->win_buffer)
		getout("failed to initialize main buffer");
	renderer->win_buffer->w = LOGIC_W;
	renderer->win_buffer->h = LOGIC_H;
	renderer->win_buffer->pixels = (uint32_t *)malloc(sizeof(uint32_t) * LOGIC_H * LOGIC_W);
	if (!renderer->win_buffer->pixels)
		getout("Failed to allocate pixel buffer");
	renderer->win_buffer->pitch = LOGIC_W;
	bzero(renderer->win_buffer->pixels, sizeof(uint32_t) * LOGIC_H * LOGIC_W);
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		getout(SDL_GetError());
	renderer->win = SDL_CreateWindow(WIN_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_W, WIN_H, 0);
	if (!renderer->win)
		getout(SDL_GetError());
	renderer->rend = SDL_CreateRenderer(renderer->win, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer->rend)
		getout(SDL_GetError());
	renderer->win_tex = SDL_CreateTexture(renderer->rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LOGIC_W, LOGIC_H);
	if (!renderer->win_tex)
		getout(SDL_GetError());
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");


	jump->player = init_player();
	jump->spear  = init_spear();

	renderer->run = TRUE;
}

static void	cleanup(t_rend *renderer)
{
	SDL_DestroyTexture(renderer->win_tex);
	SDL_DestroyRenderer(renderer->rend);
	SDL_DestroyWindow(renderer->win);
	free(renderer->win_buffer->pixels);
	free(renderer->win_buffer);
	Mix_Quit();
	SDL_Quit();
}

void draw_2_window(t_rend *rend)
{
    if (SDL_UpdateTexture(rend->win_tex, NULL, rend->win_buffer->pixels, LOGIC_W * 4) < 0)
		getout(SDL_GetError());

    SDL_RenderCopy(rend->rend, rend->win_tex, NULL, NULL);
    SDL_RenderPresent(rend->rend);
}

void	keyevent(SDL_Event *e, t_rend *rend, t_jump *jump)
{
	while (SDL_PollEvent(e))
	{
		if (e->window.event == SDL_WINDOWEVENT_CLOSE || e->key.keysym.sym == SDLK_ESCAPE)
			rend->run = FALSE;
	}

	static int old_keys;
	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_UP])	
		jump->press_keys |= K_UP;
	else if(keys[SDL_SCANCODE_DOWN])	
		jump->press_keys |= K_DOWN;
	if(keys[SDL_SCANCODE_LEFT])	
		jump->press_keys |= K_LEFT;
	else if(keys[SDL_SCANCODE_RIGHT])	
		jump->press_keys |= K_RIGHT;
	if(keys[SDL_SCANCODE_SPACE] | keys[SDL_SCANCODE_X])	
		jump->press_keys |= K_SPEAR;
	if(keys[SDL_SCANCODE_Z] | keys[SDL_SCANCODE_UP])	
		jump->press_keys |= K_JUMP;

	int new_presses = jump->press_keys & ~old_keys;
	jump->fresh_keys |= new_presses;
	old_keys = jump->press_keys;
}

// This function made by chat gpt to eliminate the jitter that showed up in my own fps_counter() implementation
void fps_counter(int ticks_this_frame) 
{
	static int frames = 0;
	static int ticks = 0;
	static double acc = 0.0;
	static uint64_t last = 0;

	uint64_t now = SDL_GetPerformanceCounter();
	if (last == 0)
	{
		last = now;
		return;
	}

	double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
	last = now;

	acc += dt;
	frames++;
	ticks += ticks_this_frame;

	if (acc >= 1.0)
	{
 		printf("%dFPS\t%dTicksPS\n", frames, ticks);
 		acc -= 1.0;   // ← IMPORTANT: subtract, don’t reset
		frames = 0;
		ticks = 0;
	}
}

int 	approach(int current_velo, int target_velo, int step)
{
	if (current_velo < target_velo)	
		return ((current_velo + step > target_velo) ? target_velo : current_velo + step);
	if (current_velo > target_velo)
		return ((current_velo - step < target_velo) ? target_velo : current_velo - step);
	return (current_velo);
}

t_point		clamp_velocity(int top_velocity, t_point velocity)
{
	t_point ret;
	ret.x = ft_clamp(-top_velocity, top_velocity, velocity.x);
	ret.y = ft_clamp(-top_velocity, top_velocity, velocity.y);
	return (ret);
}

void	update_player_velocity(t_jump *jump, int speed, int top_velocity)
{

	t_point	target_speed = {0, 0};

	if((jump->fresh_keys  & K_JUMP) == K_JUMP && jump->player.jumps)
	{
		jump->player.vel.y = approach(jump->player.vel.y, -top_velocity, 64);
		jump->player.jumps--;
	}
//	if(jump->k.d)
	if((jump->press_keys  & K_LEFT) == K_LEFT)
	{
		target_speed.x = -top_velocity;
		jump->player.dir = 1;
	}
	if((jump->press_keys  & K_RIGHT) == K_RIGHT)
	{
		target_speed.x = top_velocity;
		jump->player.dir = 0;
	}

	jump->player.vel.x = approach(jump->player.vel.x, target_speed.x, speed);
}

// TODO Replace the hardcoded subpixels value with constant.
// >> 5 = 32 subpixels. >> 4 = 16 subpixels
static inline t_point	world_point_to_rend_point(t_point point)
{
	t_point ret;

	ret.x = (point.x >> 4) + (LOGIC_W >> 1); 
	ret.y = (point.y >> 4) + (LOGIC_H >> 1); 

	return (ret);
}

t_point	point_add(t_point a, t_point b)
{
	t_point ret;

	ret.x = a.x + b.x;
	ret.y = a.y + b.y;

	return (ret);
}


//	if we go under the floor, set height to floor and velocity.y to zero
void		collision(t_obj *obj)
{
	if (obj->pos.y > (100 - (obj->size.y >> 1)) << 4) 		// Floor
	{
		obj->pos.y = (100 - (obj->size.y >> 1)) << 4;
		obj->vel.y = 0;
		if(obj->type == TYPE_PLAYER)
			obj->jumps = obj->max_jumps;
	}
	if (obj->pos.y < -(150 << 4) + ((obj->size.y >> 1) << 4)) // Ceiling
	{
		obj->pos.y = -(150 << 4) + (8 << 4);
		obj->vel.y = -obj->vel.y >> 1;
	}

	if (obj->pos.x < -(200 << 4) + ((obj->size.x >> 1) << 4)) // Left wall
	{
		if (obj->type == TYPE_SPEAR && abs(obj->vel.x) > 100) // if thrown fast, spear gets stuck
		{
			obj->stuck = TRUE;
			obj->vel = (t_point){0, 0};
		}
		else												// else bounces
			obj->vel.x = -obj->vel.x >> 1;
		obj->pos.x = -(200 << 4) + ((obj->size.x >> 1)  << 4);
	}
	if (obj->pos.x > (200 << 4) - (((obj->size.x >> 1) + 1) << 4)) // Right wall
	{
		if (obj->type == TYPE_SPEAR && abs(obj->vel.x) > 100)
		{
			obj->stuck = TRUE;
			obj->vel = (t_point){0, 0};
		}
		else
			obj->vel.x = -obj->vel.x >> 1;
		obj->pos.x = (200 << 4) - (((obj->size.x >> 1) + 1)  << 4);
	}
}

t_point		gravity(t_point player_pos)
{
	player_pos.y = approach(player_pos.y, 128, 4);
	return(player_pos);
}

//	replace magic numbers with constants
void cape(t_rend *rend, t_point player_pos)
{
	static	t_point cape[30];
	int		i = 29;

	while(i > 0)
	{
		cape[i] = cape[i - 1];
		i--;
	}
	cape[0] = player_pos;
	i = 0;
	while (i < 29)
	{
		draw_line(rend->win_buffer, world_point_to_rend_point(cape[i]), world_point_to_rend_point(cape[i +1]), 0xFFFF0000);
		i++;
	}

//	draw_circle(rend->win_buffer, world_point_to_rend_point(cape[i]), 8, 0xFFFFFFFF);	

}

bool	is_in_range(int pos_a, int pos_b, int	range)
{
	if (pos_a < pos_b + range && pos_a > pos_b - range)
		return (TRUE);
	return (FALSE);

}

bool	is_in_range_2d(t_point pos_a, t_point pos_b, t_point	range)
{
	if (is_in_range(pos_a.x, pos_b.x, range.x) && is_in_range(pos_a.y, pos_b.y, range.y))
		return (TRUE);
	return (FALSE);
}

// Change logic for input detection once fresh_press and press_press for keyevents is implemented
void spear_interaction(t_jump *jump)
{
	static t_point	spear_lag[2];
	static bool		fresh_pick		= FALSE;
	static bool		from_charge		= FALSE;
	t_point			pickup_range	= {320, 160};
	static int		charge_timer 	= 0;
	int				time_to_throw 	= 30;

	if((jump->press_keys & K_SPEAR) && jump->spear.held == FALSE && is_in_range_2d(jump->player.pos, jump->spear.pos, pickup_range))
	{
		jump->spear.held = TRUE;
		fresh_pick = TRUE;
		spear_lag[0] = jump->player.pos;
		jump->spear.stuck = FALSE;
	}
	else if((jump->press_keys & K_SPEAR) && jump->spear.held == TRUE && fresh_pick == FALSE)
	{
		charge_timer++;
		from_charge = TRUE;
	}
	else if(!(jump->press_keys & K_SPEAR))
	{
		if (from_charge == TRUE)
		{
			if(charge_timer >= time_to_throw)
			{
				if(jump->player.dir)
					jump->spear.vel.x = approach(jump->spear.vel.x, -512, 512);
				else
					jump->spear.vel.x = approach(jump->spear.vel.x, 512, 512);
			}
			jump->spear.held = FALSE;
			charge_timer = 0;
			from_charge = FALSE;
		}
		fresh_pick = FALSE;
	}

	if (jump->spear.held == TRUE )
	{
		spear_lag[1] = spear_lag[0];
		spear_lag[0] = jump->player.pos;
		jump->spear.pos = spear_lag[1];
		jump->spear.vel = jump->player.vel;
	}
	else
		jump->spear.vel.x = approach(jump->spear.vel.x, 0, 1);

}

void	clear_input_masks(t_jump *jump)
{
	jump->press_keys = 0; //clean inputs after tick
	jump->fresh_keys = 0; //clean inputs after tick
}

t_obj init_player()
{
	t_obj player;

	bzero(&player, sizeof(t_obj));
	player.vel 		= (t_point){0, 0};
	player.pos 		= (t_point){0, 0};
	player.rend_pos	= (t_point){0, 0};
	player.size 	= (t_point){16, 16};
	player.dir 		= 0;
	player.type 	= TYPE_PLAYER;
	player.max_jumps= 1;
	player.jumps	= 1;

	return (player);	
}

t_obj init_spear()
{
	t_obj spear;

	bzero(&spear, sizeof(t_obj));
	spear.pos		= (t_point){1000, -1000};
	spear.vel		= (t_point){0, 0};
	spear.rend_pos 	= (t_point){0, 0};
	spear.size		= (t_point){40, 1};
	spear.held		= 0;
	spear.type 		= TYPE_SPEAR;

	return (spear);
}

void	player_logic(t_jump *jump, int accel, int top_velocity)
{
	update_player_velocity(jump, accel, top_velocity);
	jump->player.vel		= gravity(jump->player.vel);
	jump->player.pos 		= point_add(jump->player.pos, jump->player.vel);
							  collision(&jump->player);
	jump->player.rend_pos 	= world_point_to_rend_point(jump->player.pos);
}

void	spear_logic(t_jump *jump)
{
	if(jump->spear.stuck == FALSE)
		jump->spear.vel		= gravity(jump->spear.vel);
	jump->spear.pos			= point_add(jump->spear.pos, jump->spear.vel);
							  spear_interaction(jump);	
							  collision(&jump->spear);
	jump->spear.rend_pos	= world_point_to_rend_point(jump->spear.pos);
}

void	draw_spear(t_rend *rend, t_jump *jump)
{
	draw_line(rend->win_buffer, (t_point){jump->spear.rend_pos.x - (jump->spear.size.x >> 1), jump->spear.rend_pos.y}, (t_point){jump->spear.rend_pos.x + (jump->spear.size.x >> 1), jump->spear.rend_pos.y}, 0xFFFFFFFF);	
}

void	game_logic(t_rend *rend, t_jump *jump)
{
	int		accel				= 2;
	int		top_velocity		= 96; // this should be divideble by accel to avoid stutter

	player_logic(jump, accel, top_velocity);
	draw_circle(rend->win_buffer, jump->player.rend_pos, 8, 0xFFFFFFFF); //	render player

	cape(rend, jump->player.pos);

	spear_logic(jump);
	draw_spear(rend, jump);

	clear_input_masks(jump);
}

static void	loop(t_rend *rend, SDL_Event *e, t_jump *jump)
{
	uint64_t		now = 0;
	static uint64_t	last = 0;
	static double 	accumulator = 0.0; // How much real time has passed that has not yet been simulated by game logic.
	double			frame_time = 0;
	const double 	tick_duration = 1.0 / 60.0; //60Hz
	int				new_ticks = 0;

	now = SDL_GetPerformanceCounter();
	if(last == 0)
		last = now;
	
	frame_time = (double)(now - last) / SDL_GetPerformanceFrequency();
	last = now;	
	if (frame_time > 0.25)	// deathloop protection
		frame_time = 0.25;

	accumulator += frame_time;
	keyevent(e, rend, jump);
   	while (accumulator >= tick_duration)
	{
		bzero(rend->win_buffer->pixels, LOGIC_H * LOGIC_W * sizeof(uint32_t));
		game_logic(rend, jump);
		accumulator -= tick_duration;
		new_ticks++;
	}
	if(new_ticks != 0)
		draw_2_window(rend);
	fps_counter(new_ticks);
	SDL_Delay(1);
	
}

/////// testaa gprofilla
//		gprof käyttää -pg ja -g flageja, ota ne pois kun ei enää tarvita
int	main(void)
{
	t_rend		rend;
	SDL_Event	e;
	t_jump		jump;

	init(&rend, &jump);
	while (rend.run)
		loop(&rend, &e, &jump);
	cleanup(&rend);
	return (0);
}
