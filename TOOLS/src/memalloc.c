/*********************************************************
**	MEMALLOC.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"

int	memalloc( freq_num, time_num, vis_r_ptr,	vis_i_ptr )
	int		freq_num;		/* Number of Frequency				*/
	int		time_num;		/* Number of Time					*/
	int		*vis_r_ptr;		/* Pointer of Real Data Storage		*/
	int		*vis_i_ptr;		/* Pointer of Real Data Storage		*/
{
	float	*real_ptr;		/* Pointer of Real Data Storage		*/
	float	*imag_ptr;		/* Pointer of Imag Data Storage		*/
/*
---------------------------------------- ACCESS TO SHARED MEMORY
*/

	real_ptr	= (float *)malloc(freq_num*time_num*sizeof(float));
	imag_ptr	= (float *)malloc(freq_num*time_num*sizeof(float));

	if( (real_ptr == NULL) || (imag_ptr == NULL) ){
        return(-1);
    }

	*vis_r_ptr  = (int)real_ptr;
	*vis_i_ptr  = (int)imag_ptr;

	return(0);
}
