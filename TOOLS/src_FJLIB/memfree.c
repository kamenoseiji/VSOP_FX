/*********************************************************
**	MEMFREE.C	: Release Memory Area					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"

int	memfree( vis_r_ptr,	vis_i_ptr )
	int		*vis_r_ptr;		/* Pointer of Real Data Storage		*/
	int		*vis_i_ptr;		/* Pointer of Real Data Storage		*/
{
	float	*real_ptr;		/* Pointer of Real Data Storage		*/
	float	*imag_ptr;		/* Pointer of Imag Data Storage		*/
/*
---------------------------------------- ACCESS TO SHARED MEMORY
*/
	real_ptr = (float *)(*vis_r_ptr);
	imag_ptr = (float *)(*vis_r_ptr);

	free(real_ptr);
	free(imag_ptr);

	return(0);
}
