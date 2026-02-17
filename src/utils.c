#include "jumper.h"



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

void backend_checker(t_rend *rend)
{
	// check for gpu
	SDL_RendererInfo info;
	SDL_GetRendererInfo(rend->rend, &info);
	printf("Renderer backend: %s\n", info.name);
}

int 	approach(int current_velo, int target_velo, int step)
{
	if (current_velo < target_velo)	
		return ((current_velo + step > target_velo) ? target_velo : current_velo + step);
	if (current_velo > target_velo)
		return ((current_velo - step < target_velo) ? target_velo : current_velo - step);
	return (current_velo);
}

//	the + 8 moves the center of the world point to the center of the pixel
t_point	world_point_to_rend_point(t_point point)
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


float	lerp_1d(float start, float end, float t)
{
		return (start + t * (end - start));
}


float my_max(int a, int b)
{
	if(a > b)
		return(a);
	return(b);
}

int		ft_clamp(int min, int max, int nb)
{
	if (nb < min)
		return (min);
	if (nb > max)
		return (max);
	return (nb);
}

// avoid using as it can cause jitter. Use approach instead.
t_point		clamp_velocity(int top_velocity, t_point velocity)
{
	t_point ret;
	ret.x = ft_clamp(-top_velocity, top_velocity, velocity.x);
	ret.y = ft_clamp(-top_velocity, top_velocity, velocity.y);
	return (ret);
}

