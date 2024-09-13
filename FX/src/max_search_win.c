/*********************************************************
**	MAX_SEARCH_WIN.C: Global Fringe Search using		**
**					CODA File System					**
**					with Limited Search Window			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	max_search_win( amp_ptr, valid_ss, x_num, y_num,
				init_win_x,		width_win_x,
				init_win_y,		width_win_y,
				amp_max_ptr, x_max_ptr, y_max_ptr )
	float	*amp_ptr;			/* Initial Pointer of Amplitude Map	*/
	int		valid_ss;			/* Valid Number of SS				*/
	int		x_num;				/* Total Pixels in X-Axis			*/
	int		y_num;				/* Total Pixels in Y-Axis			*/
	int		init_win_x;			/* Search Start Pixel Num			*/
	int		width_win_x;		/* Search Window Width				*/
	int		init_win_y;			/* Search Start Pixel Num			*/
	int		width_win_y;		/* Search Window Width				*/
	float	*amp_max_ptr;		/* Peak Amplitude					*/
	int		*x_max_ptr;			/* Peak Position					*/
	int		*y_max_ptr;			/* Peak Position					*/
{
	int		x_index;
	int		y_index;

	/*-------- CHECH CONDITIONS about WINDOW POSITION and SIZE --------*/
	if(init_win_x < 0){	init_win_x = 0;	}
	if(init_win_y < 0){	init_win_y = 0;	}
	if(init_win_x > x_num){	init_win_x = x_num;	}
	if(init_win_y > y_num){	init_win_x = y_num;	}
	if(width_win_x < 0){width_win_x = 1;}
	if(width_win_y < 0){width_win_y = 1;}
	if(width_win_x > x_num){width_win_x = x_num;}
	if(width_win_y > y_num){width_win_y = y_num;}
	if(init_win_x + width_win_x > x_num){width_win_x = x_num - init_win_x;}
	if(init_win_y + width_win_y > x_num){width_win_y = y_num - init_win_y;}

	*amp_max_ptr	= -9999.0;
	for(y_index=0; y_index<y_num; y_index++){
		for(x_index=0; x_index<x_num; x_index++){
			*amp_ptr /= valid_ss;

			if(    (x_index >= init_win_x )
				&& (x_index < (init_win_x + width_win_x) )
				&& (y_index >= init_win_y )
				&& (y_index < (init_win_y + width_win_y) )
				&& (*amp_ptr > *amp_max_ptr) ){
				*amp_max_ptr	= *amp_ptr;
				*x_max_ptr = x_index - x_num/2;
				*y_max_ptr = y_index - y_num/2;
			}

			amp_ptr++;
		}
	}
	return(0);
}
