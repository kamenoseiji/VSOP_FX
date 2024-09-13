/*********************************************************
**	CAL_EXEC.C : Scaling for Reference Antenna Gain		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	cal_exec( freq_num, vis_r_ptr, vis_i_ptr, bp_r_ptr, bp_i_ptr, vis_max_ptr )
	int		freq_num;
	double	*vis_r_ptr;
	double	*vis_i_ptr;
	double	*bp_r_ptr;
	double	*bp_i_ptr;
	double	*vis_max_ptr;
{
	int		freq_index;
	double	bp_sqr;
	double	visamp;
	double	vis_r, vis_i;

	*vis_max_ptr	= -9999.0;
	for(freq_index=0; freq_index<freq_num; freq_index++){
		vis_r	= *vis_r_ptr;
		vis_i	= *vis_i_ptr;

		bp_sqr	= (*bp_r_ptr)*(*bp_r_ptr) + (*bp_i_ptr)*(*bp_i_ptr);

		*vis_r_ptr	= (vis_r*(*bp_r_ptr) + vis_i*(*bp_i_ptr)) / bp_sqr;
		*vis_i_ptr	= (vis_i*(*bp_r_ptr) - vis_r*(*bp_i_ptr)) / bp_sqr;

		visamp	= sqrt((*vis_r_ptr)*(*vis_r_ptr) + (*vis_i_ptr)*(*vis_i_ptr));

		if( visamp > *vis_max_ptr ){
			*vis_max_ptr = visamp;
		}
		vis_r_ptr++;	vis_i_ptr++;	bp_r_ptr++;	bp_i_ptr++;
	}
	return(0);
}
