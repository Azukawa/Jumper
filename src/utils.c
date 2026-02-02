#include "jumper.h"

void backend_checker(t_rend *rend)
{
	// check for gpu
	SDL_RendererInfo info;
	SDL_GetRendererInfo(rend->rend, &info);
	printf("Renderer backend: %s\n", info.name);
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


