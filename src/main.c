#include "snaker.h"

//TODO
//Better color handling: Perhaps color_lerp() from tail to head
//better key press handling, only one change per press
//better color fadeout with distance. brightness is distance / 4
//pause button
//change rendering modes with button
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

static void	init(t_rend *renderer, t_snake *snake)
{
	bzero(snake, sizeof(t_snake));

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
	snake->k.keys = snake->k.keys | K_SPACE;
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

void	keyevent(SDL_Event *e, t_rend *rend, t_snake *snake)
{
	while (SDL_PollEvent(e))
	{
		if (e->window.event == SDL_WINDOWEVENT_CLOSE || e->key.keysym.sym == SDLK_ESCAPE)
			rend->run = FALSE;
		if (e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
			snake->k.u = 1;
		else if (e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYUP)
			snake->k.u = 0;
		if (e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
			snake->k.d = 1;
		else if (e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYUP)
			snake->k.d = 0;
		if (e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
			snake->k.l = 1;
		else if (e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYUP)
			snake->k.l = 0;
		if (e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
			snake->k.r = 1;
		else if (e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYUP)
			snake->k.r = 0;
		if (e->key.keysym.sym == SDLK_SPACE && e->type == SDL_KEYDOWN)
			snake->k.keys = ~snake->k.keys & K_SPACE;
		if (e->key.keysym.sym == SDLK_n && e->type == SDL_KEYDOWN && e->key.repeat == 0)
			snake->k.keys = snake->k.keys | 2;
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

// Calculate location of 3d point on the 2d screen
t_point	perspective(t_v3 pos)
{
	t_point ret;
	ret.x = ((pos.x / pos.z) + 1) * 0.5 * LOGIC_W;
	ret.y = (1 - (((pos.y / pos.z) + 1)) * 0.5) * LOGIC_H;
	return (ret);
}

int		color_brightness(int color, int brightness)
{
	if(brightness > 255)
		brightness = 255;
	if (brightness < 0)
		brightness = 0;

	uint8_t r, g, b;
	r = ((color >> 16 & 0xff) * brightness) >> 8;
	g = ((color >> 8 & 0xff) * brightness) >> 8;
	b = ((color  & 0xff) * brightness) >> 8;
	return(r << 16 | g << 8 | b);
}

static inline void	v3_swap(t_v3 *a, t_v3 *b)
{
	t_v3 temp;
	temp = *a;
	*a = *b;
	*b = temp;
}
static inline void	int_swap(int *a, int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

// Sort body elements according their distance to the camera for rendering in right order
void	sort_body(t_body* snake_body, int snake_len)
{
	int i = 0;

	while(i < snake_len - 1)
	{
		if (snake_body[i].pos.z < snake_body[i + 1].pos.z)
		{
			v3_swap(&snake_body[i].pos, &snake_body[i + 1].pos);
			int_swap(&snake_body[i].order, &snake_body[i + 1].order);
			i = 0;
		}
		else
			i++;
	}
}

//	Nudge each element in snake_body by one to make room for new head
void nudge_body(t_v3 *snake_body, int snake_len)
{
	int index = snake_len - 2;	// is -2 right?
	while(index >= 0)
	{
		snake_body[index + 1].x = snake_body[index].x;
		snake_body[index + 1].y = snake_body[index].y;
		snake_body[index + 1].z = snake_body[index].z;
		index--;
	}

}

// Draw each segment of snake on screen
void	render_body(t_rend *rend, t_body *snake_body, int snake_len)
{
	int 			index			= 0;
	int				body_size		= 50;
	int				seg_size		= 0;
	float			size_increment	= (float)body_size / snake_len;
	int				col				= 0xff0000;
	int				seg_col			= 0;

	while(index < snake_len)
	{
			t_point pos_on_screen = perspective(snake_body[index].pos);	

			seg_size = (body_size - (size_increment * snake_body[index].order)) / snake_body[index].pos.z;
			seg_col = color_brightness(col + (snake_len - snake_body[index].order) * 1500, 255 - snake_body[index].pos.z * 28);
			draw_circle(rend->win_buffer, pos_on_screen, seg_size, seg_col);
			index++;
	}

}

// Fill the lower resolution rend_body used for rendering from the high resolution snake_body
void	fill_rend_body(t_v3 *snake_body, t_body *rend_body, int snake_len)
{
	int snake_index = 0;
	int rend_index = 0;

	while (snake_index < snake_len)
	{
		if(snake_index % 15 == 0)
		{
			rend_body[rend_index].pos = snake_body[snake_index];
			rend_body[rend_index].order = rend_index;
			
			rend_index++;
		}
		snake_index++;
	}
}

// Onko nopeempia keinoja swapata/shiftata koko käärme?
// Jos laskettaiskin jokaisen segmentin positio tallentamisen sijaan?
void	draw_snake(t_rend *rend, t_v3 pos)
{
	int				snake_len		= 645;
	int				rend_len		= 43;
	static	t_v3	snake_body[645];
	t_body			rend_body[43];

	nudge_body(snake_body, snake_len);
	snake_body[0] = pos;
	fill_rend_body(snake_body, rend_body, snake_len);
	sort_body(rend_body, rend_len);
	render_body(rend, rend_body, rend_len);
	
}

t_v3	snake_movement(float speed, int mode)
{

	float	wobble = (float)sinf(speed * 0.14) + sinf( speed * 0.13) + sinf(speed * 0.012);
	if (mode == 1) // helical, up and down flying motion
	{
		t_v3	snake_head	= {0.3, 0, 6};

		snake_head.x += sin(speed / 4) * 1.3 + wobble * 0.2;
		snake_head.z += cos(speed / 4) * 1.6 + wobble * 0.2;
		snake_head.y += cos(speed / 30) * 5 + wobble * 0.05;

		return (snake_head);
	}
	else // clockwise rotating flying motion
	{
		t_v3	snake_head	= {0.3, -0.5, 4.7};

		snake_head.x += sinf(speed / 10) * 1.6 + wobble * 0.2;
		snake_head.z += cosf(speed / 10) * 4 + wobble * 0.1;
		snake_head.y += cosf(speed / 5.3) * 0.2 + wobble * 0.05;

		return (snake_head);
	}
}

bool	dir_change_x(t_point front, t_point back)
{
	bool	dir_left = FALSE;
	static bool	prev_dir_left = FALSE;
	bool	ret = FALSE;

	if(front.x < back.x)
		dir_left = TRUE;
	else
		dir_left = FALSE;
	
	if(dir_left != prev_dir_left)
			ret = TRUE;
	prev_dir_left = dir_left;
	return(ret);
}

bool	dir_change_y(t_point front, t_point back)
{
	bool	dir_up = FALSE;
	static bool	prev_dir_up = FALSE;
	bool	ret = FALSE;

	if(front.y > back.y)
		dir_up = TRUE;
	else
		dir_up = FALSE;
	
	if(dir_up != prev_dir_up)
			ret = TRUE;
	prev_dir_up = dir_up;
	return(ret);
}

void	draw_snake_2(t_rend *rend, uint32_t time, int mode, t_v3 origin, int snake_type)
{
	int	snake_len = 50;
	int	index = 0;
	int	body_size = (1.0 / 16.0) * LOGIC_W;
	int seg_size = 0;
	int prev_seg_size = 0;
	float			size_increment	= (float)body_size / snake_len;
	int	col = 0xfffffa;
	float	speed = time * 0.05;
	t_v3 pos = {0,0,0};
	t_point	prev_seg = {0, 0};

//	int		snake_type = 1;

	while(index < snake_len)
	{
			pos = v3_add(snake_movement(speed, mode), origin);
			t_point pos_on_screen = perspective(pos);	

			seg_size = (body_size - (size_increment * index)) / pos.z;
			if (snake_type == 0) // snake made from circles
				draw_circle(rend->win_buffer, pos_on_screen, seg_size, col);
			else if (snake_type == 1) //ribbon like snake
			{
				if(index == 1)
				{
					draw_line(rend->win_buffer, (t_point){pos_on_screen.x, pos_on_screen.y + seg_size}, prev_seg, col);
					draw_line(rend->win_buffer, (t_point){pos_on_screen.x, pos_on_screen.y - seg_size}, prev_seg, col);
					dir_change_x(prev_seg, pos_on_screen);
				}
				else if (index != 0)
				{
					draw_line(rend->win_buffer, (t_point){pos_on_screen.x, pos_on_screen.y + seg_size}, (t_point){prev_seg.x, prev_seg.y + prev_seg_size}, col);
					draw_line(rend->win_buffer, (t_point){pos_on_screen.x, pos_on_screen.y - seg_size}, (t_point){prev_seg.x, prev_seg.y - prev_seg_size}, col);
					if (dir_change_x(prev_seg, pos_on_screen))
					{
						// When snake changes direction from left to right, draw vertical line	
						draw_line(rend->win_buffer, (t_point){prev_seg.x, prev_seg.y + prev_seg_size}, (t_point){prev_seg.x, prev_seg.y - prev_seg_size}, col);
					}

				}
				prev_seg = pos_on_screen;
				prev_seg_size = seg_size;
			}
			else // skeleton snake
			{		
				if(index != 0)
				{
					draw_line(rend->win_buffer, pos_on_screen, prev_seg, col); // center line
					if (seg_size > 0) // draw bones only if there is size to them
					{
						t_point	temp_pos_on_screen = pos_on_screen;
						temp_pos_on_screen.y += seg_size;
						draw_line(rend->win_buffer, temp_pos_on_screen, prev_seg, col);//bottom line
						temp_pos_on_screen = pos_on_screen;
						temp_pos_on_screen.y -= seg_size;
						draw_line(rend->win_buffer, temp_pos_on_screen, prev_seg, col);
					}
				}	
				prev_seg = pos_on_screen;
				prev_seg_size = seg_size;

			}
			speed = speed - 1; // - 20 saa hienon pyörteen
			index++;
	}
}

// muuta speediä sinillä
void snake_logic(t_rend *rend, t_snake *snake)
{
	
	static uint32_t	time = 1000;
	static int	mode;
	static int	type;
	t_v3		origin_1 = {0,0,0};
	t_v3		origin_2 = {10,8,10};
	t_v3		origin_3 = {-10,8,10};
	t_v3		origin_4 = {0,0,0};

	if(snake->k.keys & K_SPACE)
		time++;
	if(snake->k.u)
		mode = ~mode & 1;
	if(snake->k.keys & K_N)
	{
		type++;
		if (type > 2)
			type = 0;
		snake->k.keys = snake->k.keys & ~K_N;
	}
	draw_snake_2(rend, time, mode, origin_1, type);	
	draw_snake_2(rend, time, 1, origin_2, type);	
	draw_snake_2(rend, time + 100000, 1, origin_3, type);	
	draw_snake_2(rend, time + 300000, 1, origin_4, type);	
//	draw_snake(rend, snake_movement(time * 0.08, mode));	
}

static void	loop(t_rend *rend, SDL_Event *e, t_snake *snake)
{
	uint64_t		now = 0;
	static uint64_t	last = 0;
	static double 	accumulator = 0.0; // How much real time has passed that has not yet been simulated by game logic.
	double			frame_time = 0;
//	const double 	tick_duration = 1.0 / 60.0; //60Hz
	const double 	tick_duration = 1.0 / 120.0; //120Hz
	int				new_ticks = 0;

	now = SDL_GetPerformanceCounter();
	if(last == 0)
		last = now;
	
	frame_time = (double)(now - last) / SDL_GetPerformanceFrequency();
	last = now;	
	if (frame_time > 0.25)	// deathloop protection
		frame_time = 0.25;

	accumulator += frame_time;
	keyevent(e, rend, snake);
   	while (accumulator >= tick_duration)
	{
		bzero(rend->win_buffer->pixels, LOGIC_H * LOGIC_W * sizeof(uint32_t));
		snake_logic(rend, snake);
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
	t_snake		snake;

	init(&rend, &snake);
	while (rend.run)
		loop(&rend, &e, &snake);
	cleanup(&rend);
	return (0);
}
