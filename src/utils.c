#include "snaker.h"

void backend_checker(t_rend *rend)
{
	// check for gpu
	SDL_RendererInfo info;
	SDL_GetRendererInfo(rend->rend, &info);
	printf("Renderer backend: %s\n", info.name);
}

t_iv3	v32iv3(t_v3 v)
{
	t_iv3	ret;

	ret.x = v.x;
	ret.y = v.y;
	ret.z = v.z;
	return (ret);
}

t_point	v2p(t_v3 v)
{
	t_point	ret;
	
	ret.x = v.x;
	ret.y = v.y;
	return (ret);
}

uint32_t	get_delta()
{
	static uint32_t	tics;
	uint32_t			pretics;

	pretics = tics;
	tics = SDL_GetTicks();
	return((tics - pretics));
}

//Calculate magnitude of vector
float v3_mag(t_v3 vec)
{
    float sum = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    return sqrtf(sum);
}

t_v3	v3_add(t_v3 a, t_v3 b)
{
	a.x = a.x + b.x;
	a.y = a.y + b.y;
	a.z = a.z + b.z;
	return(a);
}

t_v3   v_sub(t_v3 a, t_v3 b)
{
	t_v3   ret;

	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return (ret);
}
  
t_v3   v_div(t_v3 vector, float c)
{
	vector.x = vector.x / c;
	vector.y = vector.y / c;
	vector.z = vector.z / c;
	return (vector);
}
  
t_v3   v_mult(t_v3 vector, float c)
{
	vector.x = vector.x * c;
	vector.y = vector.y * c;
	vector.z = vector.z * c;
	return (vector);
}

void	v_rot_x(t_v3 *vec, float rad)
{
	t_v3	prev;

	prev.z = vec->z;
	prev.y = vec->y;
	vec->z = prev.z * cos(rad) - prev.y * sin(rad);
	vec->y = prev.z * sin(rad) + prev.y * cos(rad);
}

void	v_rot_y(t_v3 *vec, float rad)
{
	t_v3	prev;

	prev.x = vec->x;
	prev.z = vec->z;
	vec->x = prev.x * cos(rad) - prev.z * sin(rad);
	vec->z = prev.x * sin(rad) + prev.z * cos(rad);
}

void	v_rot_z(t_v3 *vec, float rad)
{
	t_v3	prev;

	prev.y = vec->y;
	prev.x = vec->x;
	vec->x = prev.x * cos(rad) - prev.y * sin(rad);
	vec->y = prev.x * sin(rad) + prev.y * cos(rad);
}

t_v3	v_rot_xyz(t_v3 vec, t_v3 rot)
{
	t_v3	ret;

	ret = vec;
	v_rot_x(&ret, DEG_TO_RAD * rot.x);
	v_rot_y(&ret, DEG_TO_RAD * rot.y);
	v_rot_z(&ret, DEG_TO_RAD * rot.z);
	return (ret);
}

t_v3	v3_cross(t_v3 va, t_v3 vb)
{
	t_v3	result;

	result.x = va.y * vb.z - va.z * vb.y;
	result.y = va.z * vb.x - va.x * vb.z;
	result.z = va.x * vb.y - va.y * vb.x;
	return (result);
}

float	v3_len(t_v3 v)
{
		return (sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
}

float   v3_dot(t_v3 va, t_v3 vb)
{
    return (va.x * vb.x + va.y * vb.y + va.z * vb.z);
}

float   v2_dot(t_v3 va, t_v3 vb)
{
    return (va.x * vb.x + va.y * vb.y);
}

t_v3	rad2vec3(float dir)
{
	t_v3	ret = {0,0,0};
	
	ret.x = cos(dir);
	ret.y = sin(dir);
	return (ret);
}
float	lerp_1d(float start, float end, float t)
{
		return (start + t * (end - start));
}

t_v3	v3_lerp(t_v3 va, t_v3 vb, float t)
{
	t_v3	ret = {0,0,0};
	
	ret.x = lerp_1d(va.x, vb.x, t);
	ret.y = lerp_1d(va.y, vb.y, t);
	ret.z = lerp_1d(va.z, vb.z, t);
	return (ret);
}


/*
*	Multiplying negative values with themselves will always result in positive
*	values, therefore sqrt() call will always success.
*	(Additionally, it does not trip onto possible NaN / Inf values.)
*/
float	v_len(t_v3 v)
{
	return (sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
}

t_v3	v_normalize(t_v3 v)
{
	t_v3	ret;
	float		l;

	l = v_len(v);
	ret.x = v.x / l;
	ret.y = v.y / l;
	ret.z = v.z / l;
	return (ret);
}

float my_max(float a, float b)
{
	if(a > b)
		return(a);
	return(b);
}

t_v3 v3_scale(t_v3 v, float scalar)
{
    t_v3 result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}

t_point		intpoint_2_point(t_intpoint p1)
{
	t_point p2;
	p2.x = (uint32_t)p1.x;
	p2.y = (uint32_t)p1.y;
	return(p2);
}

// this is dumb, make to use clamp
t_point	limit_point_to_screen(t_intpoint p)
{
	if (p.x < 0)
		p.x = 0;
	if (p.x > LOGIC_W)
		p.x = LOGIC_W;
	if (p.y < 0)
		p.y = 0;
	if (p.y > LOGIC_H)
		p.y = LOGIC_H;
	t_point p_ret;
	p_ret.x = (uint32_t)p.x;
	p_ret.y = (uint32_t)p.y;

	return(p_ret);
}

int		ft_clamp(int min, int max, int nb)
{
	if (nb < min)
		return (min);
	if (nb > max)
		return (max);
	return (nb);
}


