#include "jumper.h"

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
	int 		iterations	= 5;
	int			j 			= cape->cape_len - 1;

	if(abs(player_vel.x) > 16 || abs(player_vel.y) > 16)
	{ //  player moves, cape just follows previous player postions
		while(j > 0)
		{
			cape->link[j].old = cape->link[j].pos;
			cape->link[j] = cape->link[j - 1];
			cape->link[j].pinned = TRUE;
			j--;
		}
	}
	else	// else unpin links for physics to work on them
	{	
		j = 1;
		while(j < cape->cape_len)
		{
			cape->link[j].pinned = FALSE;
			j++;
		}
	}

	cape->link[0].pos = player_pos;
	cape->link[0].old = player_pos;
	cape->link[0].pinned = TRUE;
	integrate_points(cape->link, cape->cape_len);
	
	for(int iter_i = 0; iter_i < iterations; iter_i++)
	{
		for(int i = 0; i < cape->cape_len - 1; i++)
			distance_constraint(&cape->link[i], &cape->link[i + 1], cape->rest_len);
	}
}

void	draw_cape(t_rend *rend, t_cape *cape, t_point camera)
{
	for(int i = 0; i < cape->cape_len - 1; i++)
	{
		draw_line(rend->win_buffer, world_point_to_rend_point(point_sub(cape->link[i].pos, camera)), world_point_to_rend_point(point_sub(cape->link[i +1].pos, camera)), 0xFFFF0000);
	}
}


