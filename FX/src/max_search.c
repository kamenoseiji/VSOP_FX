/*********************************************************
**	MAX_SEARCH.C	: Global Fringe Search using		**
**					CODA File System					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	max_search( amp_ptr, valid_ss, x_num, y_num,
				amp_max_ptr, x_max_ptr, y_max_ptr )
	float	*amp_ptr;
	int		valid_ss;
	int		x_num;
	int		y_num;
	float	*amp_max_ptr;
	int		*x_max_ptr;
	int		*y_max_ptr;
{
	int		x_index;
	int		y_index;

	*amp_max_ptr	= -9999.0;
	for(y_index=0; y_index<y_num; y_index++){
		for(x_index=0; x_index<x_num; x_index++){
			*amp_ptr /= valid_ss;
			if( *amp_ptr > *amp_max_ptr ){
				*amp_max_ptr	= *amp_ptr;
				*x_max_ptr = x_index - x_num/2;
				*y_max_ptr = y_index - y_num/2;
			}
			amp_ptr++;
		}
	}
	return(0);
}
