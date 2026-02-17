#include "jumper.h"
t_obj init_player();
t_obj init_spear();
void	init_cape(t_cape *cape, t_point player_pos);
void	terrain_collision_x(t_obj *obj, t_map *map);
void	terrain_collision_y(t_obj *obj, t_map *map);
void	terrain_collision(t_obj *obj, t_map *map);

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

t_map	init_map()
{
	t_map 	map;
	map.x = 20;
	map.y = 20;
	map.map = 	"XXXXXXXXXXXXXXXXXXXX"
				"X000000000000000000X"
				"X000000000000000000X"
				"X000000000000000000X"
				"X0000X00X0000000000X"
				"X0000XXXX0000000000X"
				"X0000000000000X0000X"
				"X0000000000000X0000X"
				"X000000000000000000X"
				"X000000000000000000X"
				"X000000000000000000X"
				"X0000000000000X0000X"
				"X000000000000000000X"
				"XXXX000000000000000X"
				"X000000000000000000X"
				"X0000000000X0000000X"
				"X00000000XXXX000000X"
				"X0000X0000000000000X"
				"X0000X0000000000000X"
				"XXXXXXXXXXXXXXXXXXXX";

	map.tile_rend_size = 16;
	map.tile_world_size = map.tile_rend_size << 4;
	map.tile_size = (t_point){map.tile_world_size, map.tile_world_size};
	map.map_origo = (t_point){-1280, 0};

	return (map);
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
	jump->map	= init_map();

		init_cape(&jump->cape, jump->player.pos);
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
	static int			jump_meter = 20;

	if((jump->fresh_keys  & K_JUMP) == K_JUMP && jump->player.jumps)
	{
	//	jump->player.vel.y = approach(jump->player.vel.y, -top_velocity, 64);
		jump->player.vel.y = approach(0, -top_velocity, 64);
		jump->player.jumps--;
		jump_meter = 20;
	}
	else if((jump->press_keys  & K_JUMP) == K_JUMP && jump_meter)
	{
		jump->player.vel.y = approach(jump->player.vel.y, -top_velocity, 4);
		jump_meter--;
	}
	else
		jump_meter = 0;

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

//	the + 8 moves the center of the world point to the center of the pixel
static inline t_point	world_point_to_rend_point(t_point point)
{
	t_point ret;

	ret.x = ((point.x + 8) >> 4) + (LOGIC_W >> 1); 
	ret.y = ((point.y + 8) >> 4) + (LOGIC_H >> 1); 

	return (ret);
}

t_point	point_add(t_point a, t_point b)
{
	t_point ret;

	ret.x = a.x + b.x;
	ret.y = a.y + b.y;

	return (ret);
}

t_point	point_sub(t_point a, t_point b)
{
	t_point ret;

	ret.x = a.x - b.x;
	ret.y = a.y - b.y;

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

// this function is from think geek
int64_t square_root(int64_t n)
{
    // Find MSB(Most significant Bit) of N
    int64_t msb = (int64_t)(log2(n));

    // (a = 2^msb)
    int64_t a = 1 << msb;
    int64_t result = 0;
    while (a != 0) {
		// Check whether the current value
		// of 'a' can be added or not
		if ((result + a) * (result + a) <= n) {
			result += a;
		}

		// (a = a/2)
		a >>= 1;
    }
    // Return the result
    return result;
}


void		distance_constraint(t_link *a, t_link *b, int	desired_length)
{
	int64_t	solver_scale = 32;	// this is the resolution of simulation. Make in to define

	int64_t	delta_x = b->pos.x - a->pos.x;
	int64_t	delta_y = b->pos.y - a->pos.y;

	int64_t	distance_squared = delta_x * delta_x + delta_y * delta_y;
	if (distance_squared == 0)
		return ;
	
	int64_t	current_distance = square_root(distance_squared);
	int64_t	distance_error = current_distance - desired_length;
	
	if (distance_error == 0)
		return ;

	int64_t	scaled_dir_x = (delta_x * solver_scale) / current_distance;
	int64_t	scaled_dir_y = (delta_y * solver_scale) / current_distance;

	int64_t	half_correction = distance_error / 2; //consider bitshift

	if (a->pinned == FALSE) 
	{
		a->pos.x += (scaled_dir_x * half_correction) / solver_scale;	
		a->pos.y += (scaled_dir_y * half_correction) / solver_scale;	
	}
	if (b->pinned == FALSE) 
	{
		b->pos.x -= (scaled_dir_x * half_correction) / solver_scale;	
		b->pos.y -= (scaled_dir_y * half_correction) / solver_scale;	
	}
}

void	integrate_points(t_link *cape, int cape_len)
{
	int	gravity;
	static int	sub_counter;
//	if (sub_counter % 1 == 0)
		gravity = 2;
//	else
//		gravity = 0;
	sub_counter++;
	for (int i = 0; i <= cape_len - 1; i++)
	{
		if (cape[i].pinned == FALSE)
		{
			int vel_x = cape[i].pos.x - cape[i].old.x;
			int vel_y = cape[i].pos.y - cape[i].old.y;

			cape[i].old.x = cape[i].pos.x;
			cape[i].old.y = cape[i].pos.y;

			cape[i].pos.x += vel_x / 2;		// this division makes rope less springy
			cape[i].pos.y += vel_y  / 2 + gravity; // plussaa gravity tähän
		}
	}

}

void	init_cape(t_cape *cape, t_point player_pos)
{
	cape->cape_len = 30;
	cape->rest_len = 32;
	for (int i = 0; i < cape->cape_len; i++)
	{
		cape->link[i].pos		= player_pos;
		cape->link[i].old		= player_pos;
		cape->link[i].pinned	= FALSE;
	}
}

//	replace magic numbers with constants
void		calculate_cape(t_point player_pos, t_point player_vel, t_cape* cape)
{
	int			cape_len = cape->cape_len;
	int			rest_len = cape->rest_len;
	int 		iterations = 5;
	int		j = 29;

	if(abs(player_vel.x) > 16 || abs(player_vel.y) > 16)
	{
		while(j > 0)
		{
			cape->link[j].old = cape->link[j].pos;
			cape->link[j] = cape->link[j - 1];
			cape->link[j].pinned = TRUE;
			j--;
		}
	}
	else
	{	
		j = 1;
		while(j < cape_len)
		{
			cape->link[j].pinned = FALSE;
//			cape[j].old = cape[j].pos;
			j++;
		}
	}

	cape->link[0].pos = player_pos;
	cape->link[0].old = player_pos;
	cape->link[0].pinned = TRUE;
	integrate_points(cape->link, cape_len);
	if (!cape->link[1].pinned)
	{
   		cape->link[1].pos.x += (cape->link[0].pos.x - cape->link[1].pos.x) / 4;
    	cape->link[1].pos.y += (cape->link[0].pos.y - cape->link[1].pos.y) / 4;
		cape->link[1].old = cape->link[1].pos;
	}
	
	for(int iter_i = 0; iter_i < iterations; iter_i++)
	{
		for(int i = 0; i < cape_len - 1; i++)
		{
	//		cape[i + 1].pinned = FALSE;
			distance_constraint(&cape->link[i], &cape->link[i + 1], rest_len);
		}
	}
}

void	draw_cape(t_rend *rend, t_cape *cape, t_point camera)
{
	for(int i = 0; i < cape->cape_len - 1; i++)
	{
		draw_line(rend->win_buffer, world_point_to_rend_point(point_sub(cape->link[i].pos, camera)), world_point_to_rend_point(point_sub(cape->link[i +1].pos, camera)), 0xFFFF0000);
	}
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
		jump->spear.held	= TRUE;
		fresh_pick			= TRUE;
		spear_lag[0] 		= jump->player.pos;
		jump->spear.stuck	= FALSE;
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
			jump->spear.held	= FALSE;
			charge_timer 		= 0;
			from_charge			= FALSE;
		}
		fresh_pick = FALSE;
	}

	if (jump->spear.held == TRUE )
	{
		spear_lag[1]	= spear_lag[0];
		spear_lag[0]	= jump->player.pos;
		jump->spear.pos = spear_lag[1];
		jump->spear.vel = jump->player.vel;
	}
	else
		jump->spear.vel.x = approach(jump->spear.vel.x, 0, 1);

}

void	clear_input_masks(uint32_t *fresh_keys, uint32_t *press_keys)
{
	*press_keys = 0; //clean inputs after tick
	*fresh_keys = 0; //clean inputs after tick
}

t_obj init_player()
{
	t_obj player;

	bzero(&player, sizeof(t_obj));
	player.vel 		= (t_point){0, 0};
	player.pos 		= (t_point){256, 256};
	player.rend_pos	= (t_point){0, 0};
	player.size 	= (t_point){16, 16};
	player.dir 		= 0;
	player.type 	= TYPE_PLAYER;
	player.max_jumps= 1;
	player.jumps	= player.max_jumps;

	return (player);	
}

t_obj init_spear()
{
	t_obj spear;

	bzero(&spear, sizeof(t_obj));
	spear.pos		= (t_point){256, 256};
	spear.vel		= (t_point){0, 0};
	spear.rend_pos 	= (t_point){0, 0};
	spear.size		= (t_point){40, 1};
	spear.held		= 0;
	spear.type 		= TYPE_SPEAR;

	return (spear);
}

void	spear_logic(t_jump *jump)
{
							  spear_interaction(jump);	
	if(jump->spear.stuck == FALSE && jump->spear.held == FALSE)
		jump->spear.vel		= gravity(jump->spear.vel);
	jump->spear.pos			= point_add(jump->spear.pos, jump->spear.vel);
//							  collision(&jump->spear);
						//	terrain_collision_x(&jump->spear, &jump->map);
						//	terrain_collision_y(&jump->spear, &jump->map);
							terrain_collision(&jump->spear, &jump->map);
}

void	draw_spear(t_rend *rend, t_obj *spear, t_camera *camera)
{
	spear->rend_pos	= world_point_to_rend_point(point_sub(spear->pos, camera->pos));
	draw_line(rend->win_buffer, (t_point){spear->rend_pos.x - (spear->size.x >> 1), spear->rend_pos.y}, (t_point){spear->rend_pos.x + (spear->size.x >> 1), spear->rend_pos.y}, 0xFFFFFFFF);	
}

void	draw_terrain(t_rend *rend, t_map *map, t_point *camera)
{
	t_point	a = {0, 0};
	t_point	b = {0,0};
	t_point map_origo = point_sub(map->map_origo, *camera);

	for(int y = 0; y < map->y; y++)
	{
		for(int x = 0; x < map->x; x++)
		{
			a = point_add((t_point){map->tile_size.x * x, map->tile_size.y * y}, map_origo);
			b = point_add(a, map->tile_size);
			if(map->map[x + y * map->x] == 'X')
				draw_square(world_point_to_rend_point(a), world_point_to_rend_point(b), rend->win_buffer, 0x08FF00FF);
		}
	}
}

void	terrain_collision_x(t_obj *obj, t_map *map)
{	
	int	half_obj_w = ((obj->size.x >> 1) << 4); 
	if(half_obj_w < 1)
		half_obj_w = 1;
	int	half_obj_h = ((obj->size.y >> 1) << 4);	// change this to << 3 
	if(half_obj_h < 1)
		half_obj_h = 1;
	int y = (((obj->pos.y) - map->map_origo.y) / map->tile_world_size);
	int left_x = (((obj->pos.x - half_obj_w) - map->map_origo.x) / map->tile_world_size);
	int right_x = (((obj->pos.x + half_obj_w - 1) - map->map_origo.x) / map->tile_world_size);

	//	Left wall
	if	(obj->vel.x < 0 && \
		left_x < map->x && left_x >= 0 && y < map->y && y >= 0 && \
		map->map[left_x + y * map->x] == 'X') 
	{
		t_point	a = point_add((t_point){map->tile_size.x * left_x, map->tile_size.y * y}, map->map_origo);
		t_point	b = point_add(a, map->tile_size);

		if (obj->type == TYPE_SPEAR && abs(obj->vel.x) > 100) // if thrown fast, spear gets stuck
		{
			obj->stuck = TRUE;
			obj->vel = (t_point){0, 0};
		}
		else									
			obj->vel.x = -(obj->vel.x >> 1);
		obj->pos.x = b.x + half_obj_w;
	}
	// Right wall
	else if	(obj->vel.x > 0 && \
		right_x < map->x && right_x >= 0 && y < map->y && y >= 0 && \
		map->map[right_x + y * map->x] == 'X') 
	{
		t_point	a = point_add((t_point){map->tile_size.x * right_x, map->tile_size.y * y}, map->map_origo);

		if (obj->type == TYPE_SPEAR && abs(obj->vel.x) > 100)
		{
			obj->stuck = TRUE;
			obj->vel = (t_point){0, 0};
		}
		else
			obj->vel.x = -(obj->vel.x >> 1);
		obj->pos.x = a.x - half_obj_w;
	}
}

//	player needs hitbox
void	terrain_collision_y(t_obj *obj, t_map *map)
{	
	int	half_obj_w = ((obj->size.x >> 1) << 4); 
	if(half_obj_w < 1)
		half_obj_w = 1;
	int	half_obj_h = ((obj->size.y >> 1) << 4);	// change this to << 3 
	if(half_obj_h < 1)
		half_obj_h = 1;
	int y = (((obj->pos.y) - map->map_origo.y) / map->tile_world_size);
	int x = (((obj->pos.x) - map->map_origo.x) / map->tile_world_size);
	int left_x = (((obj->pos.x - half_obj_w) - map->map_origo.x) / map->tile_world_size);
	int right_x = (((obj->pos.x + half_obj_w - 1) - map->map_origo.x) / map->tile_world_size);
	int head_y = (((obj->pos.y - half_obj_h) - map->map_origo.y) / map->tile_world_size);
	int feet_y = (((obj->pos.y + half_obj_h) - map->map_origo.y) / map->tile_world_size);

	//	Floor
	if	(obj->vel.y > 0 && \
		x < map->x && x >= 0 && \
		feet_y < map->y && feet_y >= 0 && \
		(map->map[left_x + feet_y * map->x] == 'X' || map->map[right_x + feet_y * map->x] == 'X' || map->map[x + feet_y * map->x] == 'X' )) 
	{
		t_point	a = point_add((t_point){map->tile_size.x * x, map->tile_size.y * y}, map->map_origo);
		obj->pos.y = a.y + half_obj_h;
		obj->vel.y = 0;
		if(obj->type == TYPE_PLAYER)
			obj->jumps = obj->max_jumps;
	}

//	Ceiling
	else if	(obj->vel.y < 0 && \
		x < map->x && x >= 0 && head_y < map->y && head_y >= 0 && \
		(map->map[left_x + head_y * map->x] == 'X' || map->map[right_x + head_y * map->x] == 'X')) 
	{
		t_point	a = point_add((t_point){map->tile_size.x * x, map->tile_size.y * y}, map->map_origo);
		obj->pos.y = a.y + half_obj_h;
		obj->vel.y = -obj->vel.y >> 1;
	}
}

void	terrain_collision(t_obj *obj, t_map *map)
{
		terrain_collision_x(obj, map);
		terrain_collision_y(obj, map);
}

void	player_logic(t_jump *jump, int accel, int top_velocity)
{
	update_player_velocity(jump, accel, top_velocity);
	jump->player.vel		= gravity(jump->player.vel);
	jump->player.pos 		= point_add(jump->player.pos, jump->player.vel);
	terrain_collision(&jump->player, &jump->map);
}

t_point	camera_follow(t_point pos, t_camera camera)
{
	int	speed = 96;
	int accel = 4;
	int dead_zone = 256;

	if(camera.pos.x < pos.x - dead_zone )
		camera.vel.x = approach(camera.vel.x, -speed, accel);
	else if(camera.pos.x > pos.x + dead_zone)
		camera.vel.x = approach(camera.vel.x, speed, accel);
	else
		camera.vel.x = approach(camera.vel.x, 0, accel * 3);
	if(camera.pos.y < pos.y - (dead_zone << 1))
		camera.vel.y = approach(camera.vel.y, -speed, accel);
	else if(camera.pos.y > pos.y + (dead_zone))
		camera.vel.y = approach(camera.vel.y, speed, accel);
	else
		camera.vel.y = approach(camera.vel.y, 0, accel * 3);

	return(camera.vel);

}

void	draw_player(t_rend *rend, t_obj *player, t_camera *camera)
{
	player->rend_pos 	= world_point_to_rend_point(point_sub(player->pos, camera->pos));
	draw_square((t_point){player->rend_pos.x - (player->size.x / 2), player->rend_pos.y - (player->size.y / 2)}, (t_point){player->rend_pos.x + (player->size.x / 2), player->rend_pos.y + (player->size.y / 2)}, rend->win_buffer, 0xAAAAAA);
}

void	update_camera(t_obj *player, t_camera *camera)
{
	camera->vel = camera_follow(player->pos, *camera);
	camera->pos = point_sub(camera->pos, camera->vel);
}

void	game_logic(t_jump *jump)
{
	int		accel				= 2;
	int		top_velocity		= 96; // this should be divideble by accel to avoid stutter

	player_logic(jump, accel, top_velocity);
	spear_logic(jump);
	update_camera(&jump->player, &jump->camera);

	calculate_cape(jump->player.pos, jump->player.vel, &jump->cape);
	clear_input_masks(&jump->fresh_keys, &jump->press_keys);
}

void	calculate_graphics(t_rend *rend, t_jump *jump)
{

		draw_player(rend, &jump->player, &jump->camera);
		draw_terrain(rend, &jump->map, &jump->camera.pos);	
		draw_spear(rend, &jump->spear, &jump->camera);
		draw_cape(rend, &jump->cape, jump->camera.pos);
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
		game_logic(jump);
		accumulator -= tick_duration;
		new_ticks++;
	}
	if(new_ticks != 0)
	{
		calculate_graphics(rend, jump);
		draw_2_window(rend);
	}
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
