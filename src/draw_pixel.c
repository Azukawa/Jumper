/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   draw_pixel.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eniini <eniini@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/10 14:53:41 by eniini            #+#    #+#             */
/*   Updated: 2026/01/29 23:07:49 by alero            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "snaker.h"

/*
*	Colors the pixel in given coordinates of the given buffer as the given
*	hexadecimal value [0xAARRGGBB]. (00 for AA being zero transparency)
*/
void	draw_pixel(uint32_t x, uint32_t y, t_buffer *buf, uint32_t color)
{
	uint32_t	i;

	i = (buf->w * y) + x;
	if (x < (uint32_t)buf->w && y < (uint32_t)buf->h)
		buf->pixels[i] = color;
}
