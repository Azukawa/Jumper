#ifndef SNAKER_H
# define SNAKER_H

# include <time.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdint.h> //for datatypes
# include <string.h> //for sterror
# include <errno.h> //for errno macro
# include <math.h>
# include <stdbool.h>

# include "../libSDL2/include/SDL2/SDL_mixer.h"
# include "../libSDL2/include/SDL2/SDL.h"

# define LOGIC_W 400
# define LOGIC_H 300

# define WIN_W 800
# define WIN_H 600

# define WIN_NAME "Jumper!"

# define TRUE	1
# define FALSE	0

# define DEG_TO_RAD 0.01745329251
# define RAD_TO_DEG 57.2957795131

#define K_SPEAR		0x00000001
#define K_UP		0x00000002
#define K_DOWN		0x00000004
#define K_LEFT		0x00000008
#define K_RIGHT		0x00000010
#define K_JUMP		0x00000020

#define TYPE_PLAYER 0
#define TYPE_SPEAR  1

	
typedef struct s_buffer {
	uint32_t		*pixels;
	int			w;
	int			h;
	int			pitch;
}					t_buffer;

//datatype for handling buffer/array coordinates
typedef struct s_point {  // these used to be uint, but changed for debugging. 
	int	x;
	int	y;
}				t_point;

typedef struct s_rend
{
	SDL_Renderer	*rend;
	SDL_Window		*win;
	SDL_Texture		*win_tex;
	void			*win_pixels;
	t_buffer		*win_buffer;
	int				win_pixel_pitch; // can this be deleted?
	bool			run;
}					t_rend;

typedef struct	s_obj{
	//	Common variables
	t_point		vel;		//	objects velocity
	t_point		pos;		//	objects position in world space
	t_point		rend_pos;	//	objects position on screen
	t_point		size;		//	objects width and height
	int			dir;		//	objects direction. 0 = right, 1 = left, 2 = , 4 = down
	int			type;		//	what kind of object. Player or spear?
	//	Spear variables
	bool		held;		//	is object held by player, used for spear
	bool		stuck;		//	is object stuck in wall. Used for spear
	//	Player variables
	int			jumps;		//	How many jumps left
	int			max_jumps;	//	Maximum jumps available

}				t_obj;

typedef struct	s_jump{
	
	uint32_t	fresh_keys;
	uint32_t	press_keys;
	t_obj		player;
	t_obj		spear;
}				t_jump;


void		draw_pixel(uint32_t x, uint32_t y, t_buffer *buf, uint32_t color);
void		draw_line(t_buffer *buf, t_point p0, t_point p1, uint32_t color);
void		draw_circle(t_buffer *buf, t_point p, int r, uint32_t color);
void		draw_filled_circle(t_buffer *buf, t_point p, int r, uint32_t color);
void		draw_square(t_point a, t_point b, t_buffer *buf, int color);
int			ft_clamp(int min, int max, int nb);

#endif
