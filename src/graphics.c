#include "jumper.h"


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

void	draw_player(t_rend *rend, t_obj *player, t_camera *camera)
{
	player->rend_pos 	= world_point_to_rend_point(point_sub(player->pos, camera->pos));
	draw_square((t_point){player->rend_pos.x - (player->size.x / 2), player->rend_pos.y - (player->size.y / 2)}, (t_point){player->rend_pos.x + (player->size.x / 2), player->rend_pos.y + (player->size.y / 2)}, rend->win_buffer, 0xAAAAAA);
}

void	calculate_graphics(t_rend *rend, t_jump *jump)
{

		draw_player(rend, &jump->player, &jump->camera);
		draw_terrain(rend, &jump->map, &jump->camera.pos);	
		draw_spear(rend, &jump->spear, &jump->camera);
		draw_cape(rend, &jump->cape, jump->camera.pos);
}

