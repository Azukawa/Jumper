#include "jumper.h"

//TODO
//pause button
//

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
	renderer->win_buffer->pitch = WIN_W;
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
		if (e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
			jump->k.u = 1;
		else if (e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYUP)
			jump->k.u = 0;
		if (e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
			jump->k.d = 1;
		else if (e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYUP)
			jump->k.d = 0;
		if (e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
			jump->k.l = 1;
		else if (e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYUP)
			jump->k.l = 0;
		if (e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
			jump->k.r = 1;
		else if (e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYUP)
			jump->k.r = 0;
	}
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

static inline t_point	update_player_velocity(t_jump *jump, t_point player_vel, int speed, int top_velocity)
{

	t_point	target_speed = {0, 0};

	if(jump->k.u)
		player_vel.y = approach(player_vel.y, -top_velocity, 64);
//	if(jump->k.d)
	if(jump->k.l)
		target_speed.x = -top_velocity;
	else if(jump->k.r)
		target_speed.x = top_velocity;

	player_vel.x = approach(player_vel.x, target_speed.x, speed);

	return (player_vel);
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
t_point		proto_floor(t_point player_pos, t_point *player_vel)
{
	if (player_pos.y > 100 << 4)
	{
		player_pos.y = 100 << 4;
		player_vel->y = 0;
	}
	if (player_pos.y < -(150 << 4) + (8 << 4))
	{
		player_pos.y = -(150 << 4) + (8 << 4);
		player_vel->y = -player_vel->y >> 1;
	}

	if (player_pos.x < -(200 << 4) + (8 << 4))
	{
			
		player_pos.x = -(200 << 4) + (8 << 4);
		player_vel->x = -player_vel->x >> 1;
	}
	if (player_pos.x > (200 << 4) - (9 << 4))
	{
			
		player_pos.x = (200 << 4) - (9 << 4);
		player_vel->x = -player_vel->x >> 1;
	}

	return (player_pos);
}

t_point		gravity(t_point player_pos)
{

	player_pos.y = approach(player_pos.y, 128, 4);
	return(player_pos);
}

void game_logic(t_rend *rend, t_jump *jump)
{
	static t_point player_vel 	= {0, 0};
	static t_point player_pos 	= {0, 0};
	t_point	rend_player_pos 	= {0, 0};
	int		accel				= 2;
	int		top_velocity		= 128; // this should be divideble by accel to avoid stutter

	player_vel 		= update_player_velocity(jump, player_vel, accel, top_velocity);
	player_vel		= gravity(player_vel);
	player_pos 		= point_add(player_pos, player_vel);
	player_pos		= proto_floor(player_pos, &player_vel);
	rend_player_pos = world_point_to_rend_point(player_pos);

	draw_circle(rend->win_buffer, rend_player_pos, 8, 0xFFFFFFFF);	
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
