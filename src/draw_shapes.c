/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   draw_shapes.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eniini <eniini@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/10 15:01:46 by eniini            #+#    #+#             */
/*   Updated: 2026/01/29 22:58:11 by alero            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "snaker.h"

void	ft_swap(void *a, void *b, size_t size)
{
	char	*temp;

	temp = malloc(size);
	if (temp)
	{
		memcpy(temp, a, size);
		memcpy(a, b, size);
		memcpy(b, temp, size);
		free(temp);
	}
}

static void	init_errors(t_point p0, t_point p1, int *derror, int *error)
{
	uint32_t	ydiff;

	if (p1.y > p0.y)
		ydiff = p1.y - p0.y;
	else
		ydiff = p0.y - p1.y;
	*derror = ydiff * 2;
	*error = 0;
}

static bool	init_points(t_point *p0, t_point *p1)
{
	bool		flip;
	uint32_t	xdiff;
	uint32_t	ydiff;

	flip = FALSE;
	if (p0->x > p1->x)
		xdiff = p0->x - p1->x;
	else
		xdiff = p1->x - p0->x;
	if (p0->y > p1->y)
		ydiff = p0->y - p1->y;
	else
		ydiff = p1->y - p0->y;
	if (xdiff < ydiff)
	{
		ft_swap(&p0->x, &p0->y, sizeof(int));
		ft_swap(&p1->x, &p1->y, sizeof(int));
		flip = TRUE;
	}
	if (p0->x > p1->x)
	{
		ft_swap(&p0->x, &p1->x, sizeof(int));
		ft_swap(&p0->y, &p1->y, sizeof(int));
	}
	return (flip);
}

/*
*	Bresenham's line algorithm.
*/
void	draw_line(t_buffer *buf, t_point p0, t_point p1, uint32_t color)
{
	int		derror;
	int		error;
	t_point	crawler;
	bool	flip;

	flip = init_points(&p0, &p1);
	crawler = (t_point){p0.x, p0.y};
	init_errors(p0, p1, &derror, &error);
	while (crawler.x <= p1.x)
	{
		if (!flip)
			draw_pixel(crawler.x++, crawler.y, buf, color);
		else
			draw_pixel(crawler.y, crawler.x++, buf, color);
		error += derror;
		if (error > (int)p1.x - (int)p0.x)
		{
			if (p1.y > p0.y)
				crawler.y += 1;
			else
				crawler.y += -1;
			error -= ((int)p1.x - (int)p0.x) * 2;
		}
	}
}
/*
*	Calculates points on the border of the circle in its first octant,
*	rest are determined by symmetry. draw unique points until (x = y)
*	choosing points closest to the radius of the circle.
*/
void	draw_circle(t_buffer *buf, t_point p, int r, uint32_t color)
{
	int	x;
	int	y;
	int	d;

	x = r;
	y = 0;
	d = 1 - r;
	while (x >= y)
	{
		draw_pixel(p.x + x, p.y + y, buf, color);
		draw_pixel(p.x - x, p.y + y, buf, color);
		draw_pixel(p.x + x, p.y - y, buf, color);
		draw_pixel(p.x - x, p.y - y, buf, color);
		draw_pixel(p.x + y, p.y + x, buf, color);
		draw_pixel(p.x - y, p.y + x, buf, color);
		draw_pixel(p.x + y, p.y - x, buf, color);
		draw_pixel(p.x - y, p.y - x, buf, color);
		if (d < 0)
			d += (2 * ++y) + 1;
		else
		{
			x--;
			d += (2 * (++y - x)) + 1;
		}
	}
}

/*
*	Instead of drawing individual pixels on symmetrical positions along the
*	circle's radius, we draw lines from one end to its opposite.
*/
void	draw_filled_circle(t_buffer *buf, t_point p, int r, uint32_t color)
{
	int	x;
	int	y;
	int	d;

	x = r;
	y = 0;
	d = 1 - r;
	while (x >= y)
	{
		draw_line(buf, (t_point){p.x + x, p.y + y},
			(t_point){p.x - x, p.y + y}, color);
		draw_line(buf, (t_point){p.x + x, p.y - y},
			(t_point){p.x - x, p.y - y}, color);
		draw_line(buf, (t_point){p.x + y, p.y + x},
			(t_point){p.x - y, p.y + x}, color);
		draw_line(buf, (t_point){p.x + y, p.y - x},
			(t_point){p.x - y, p.y - x}, color);
		if (d < 0)
			d += (2 * ++y) + 1;
		else
		{
			x--;
			d += (2 * (++y - x)) + 1;
		}
	}
}




/*
*	Paints a square area of pixels with given color.
*	Ignores calls given with illegal (outside of buffer region) coordinates.
*/
void	draw_square(t_point p0, t_point p1, t_buffer *buf, int color)
{
	int	x;
	int	y;

	if ((p0.x > buf->w || p1.x > buf->w) || (p0.y > buf->h || p1.y > buf->h))
		return ;
	if (p1.x < p0.x)
		ft_swap(&p1.x, &p0.x, sizeof(int));
	if (p1.y < p0.y)
		ft_swap(&p1.y, &p0.y, sizeof(int));
	x = p0.x;
	y = p0.y;
	while (y <= p1.y)
	{
		while (x <= p1.x)
		{
			draw_pixel(x, y, buf, color);
			x++;
		}
		x = p0.x;
		y++;
	}
}

